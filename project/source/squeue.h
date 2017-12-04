// @Copyright 2017 Scott Leland Crossen

#include <pthread.h>

#pragma once

typedef enum {FULL_RESIZE, FULL_WAIT, FULL_IGNORE, FULL_ABORT} full_behavior_t;

typedef enum {EMPTY_WAIT, EMPTY_IGNORE, EMPTY_ABORT} empty_behavior_t;

typedef void* storeable;

typedef struct queue_node_t {
  storeable             value;
  struct queue_node_t*  next;
} queue_node_t;

typedef struct linked_queue_t {
  queue_node_t*         head;
  queue_node_t*         tail;
  struct queue_node_t*  allocated;
  full_behavior_t       full_behavior;
  empty_behavior_t      empty_behavior;
  pthread_cond_t        readable;
  pthread_cond_t        writeable;
  pthread_mutex_t       lock;
} linked_queue_t;

linked_queue_t* createLinkedQueue(unsigned int size, full_behavior_t full_behavior, empty_behavior_t empty_behavior);

void destroyLinkedQueue(linked_queue_t* this);

void enqueueLinked(linked_queue_t* this, storeable data);

int dequeueLinked(linked_queue_t* this, storeable* data);
