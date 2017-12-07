// @Copyright 2017 Scott Leland Crossen

#include "squeue.h"
#include "csapp.h"

#define REQ_QUEUE_INIT_SIZE 15

typedef struct fd_queue_t {
  linked_queue_t* queue;
  // Not sure if the queue wait behavior works so I'll just use a semaphore here
  sem_t           entries;
} fd_queue_t;

fd_queue_t* fd_queue;

void queue_fd(int connfd);

int dequeue_fd(int* fd);

void init_fd();
