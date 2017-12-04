// @Copyright 2017 Scott Leland Crossen

#include "squeue.h"

#define REQ_QUEUE_INIT_SIZE 16

typedef linked_queue_t fdqueue_t;

void queue_fd(fdqueue_t* fdqueue, int connfd);

int dequeue_fd(fdqueue_t* fdqueue, int* fd);

fdqueue_t* init_fdqueue();
