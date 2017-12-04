// @Copyright 2017 Scott Leland Crossen

#include <stdio.h>
#include <stdlib.h>
#include "squeue.h"

void resizeLinked(linked_queue_t* this);
int handleLinkedEmpty(linked_queue_t* this);
int handleLinkedFull(linked_queue_t* this);

linked_queue_t* createLinkedQueue(unsigned int size, full_behavior_t full_behavior, empty_behavior_t empty_behavior) {
	linked_queue_t* this = malloc(sizeof(linked_queue_t));
  if (size > 0) {
    this->allocated = malloc(sizeof(queue_node_t));
    this->tail=this->allocated;
    unsigned int iter = 0;
    while (++iter < size) {
      this->allocated->next = malloc(sizeof(queue_node_t));
      this->allocated = this->allocated->next;
    }
  }
  this->full_behavior = full_behavior;
  this->empty_behavior = empty_behavior;

  if(pthread_mutex_init(&this->lock, NULL) != 0){
    fprintf(stderr, "Could not initialize mutex\r\n");
    abort();
  }
  if(pthread_cond_init(&this->readable, NULL) != 0){
    fprintf(stderr, "Could not initialize readable condition variable\r\n");
    abort();
  }
  if(pthread_cond_init(&this->writeable, NULL) != 0){
    fprintf(stderr, "Could not initialize writable condition variable\r\n");
    abort();
  }

	return this;
}

void destroyLinkedQueue(linked_queue_t* this) {
  queue_node_t* current = (this->head != NULL) ? this->head : (this->tail != NULL) ? this->tail : NULL;
  while (current != NULL) {
    queue_node_t* next = current->next;
    free(current);
    current = next;
  }

  if(pthread_mutex_destroy(&this->lock) != 0){
    fprintf(stderr, "Could not destroy mutex\r\n");
    abort();
  }
  if(pthread_cond_destroy(&this->readable) != 0){
    fprintf(stderr, "Could not destroy readable condition variable\r\n");
    abort();
  }
  if(pthread_cond_destroy(&this->writeable) != 0){
    fprintf(stderr, "Could not destroy writable condition variable\r\n");
    abort();
  }

  free(this);
  this = NULL;
}

void enqueueLinked(linked_queue_t* this, storeable data) {
  pthread_mutex_lock(&this->lock);
  if (this->allocated == NULL) {
    if (handleLinkedFull(this) != 0) {
      pthread_mutex_unlock(&this->lock);
      return;
    }
    this->head = this->allocated;
    this->tail = this->allocated;
  } else if (this->head == NULL) {
    this->head = this->tail;
  } else if (this->tail == this->allocated) {
    if (handleLinkedFull(this) != 0) {
      pthread_mutex_unlock(&this->lock);
      return;
    }
    this->tail = this->allocated;
  } else {
    this->tail = this->tail->next;
  }
  this->tail->value = data;
  pthread_cond_signal(&this->readable);
  pthread_mutex_unlock(&this->lock);
}

int dequeueLinked(linked_queue_t* this, storeable* data) {
  pthread_mutex_lock(&this->lock);
  if(this->head == NULL) {
    if(handleLinkedEmpty(this) != 0) {
      *data = 0;
      pthread_mutex_unlock(&this->lock);
      return -1;
    }
  }
  *data = this->head->value;
  if (this->head == this->tail) {
    this->head = NULL;
  } else {
    this->allocated->next = this->head;
    this->allocated = this->allocated->next;
    this->head = this->head->next;
  }
  this->allocated->next = NULL;
  pthread_cond_signal(&this->writeable);
  pthread_mutex_unlock(&this->lock);
  return 0;
}

int handleLinkedEmpty(linked_queue_t* this) {
  switch (this->empty_behavior) {
    case EMPTY_WAIT:
      //printf("Queue empty: Waiting for data before removing\r\n");
      while(this->head == NULL) {
        pthread_cond_wait(&this->writeable, &this->lock);
      }
      return 0;
    case EMPTY_IGNORE:
      //printf("Queue empty: Returning null\r\n");
      return -1;
    case EMPTY_ABORT:
      //printf("Queue empty: Aborting\r\n");
      fprintf(stderr, "queue empty\r\n");
      abort();
    default:
      //printf("Queue empty: Unknown behavior type - Ignoring\r\n");
      return -2;
  }
}

int handleLinkedFull(linked_queue_t* this) {
  switch (this->full_behavior) {
    case FULL_RESIZE:
      //printf("Queue full: Resizing queue\r\n");
      resizeLinked(this);
      return 0;
    case FULL_WAIT:
      //printf("Queue full: Waiting for space before adding\r\n");
      while (this->tail == this->allocated) {
        pthread_cond_wait(&this->writeable, &this->lock);
      }
      return 0;
    case FULL_IGNORE:
      //printf("Queue full: Dropping added data\r\n");
      return -1;
    case FULL_ABORT:
      //printf("Queue full: Aborting\r\n");
      fprintf(stderr, "queue full\r\n");
      abort();
    default:
      //printf("Queue full: Unknown behavior type - dropping\r\n");
      return -2;
  }
}

void resizeLinked(linked_queue_t* this) {
  queue_node_t* new = malloc(sizeof(queue_node_t));
  if (this->allocated == NULL) {
    this->allocated = new;
  } else {
    this->allocated->next = new;
    this->allocated = this->allocated->next;
  }
}
