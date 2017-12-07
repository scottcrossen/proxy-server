// @Copyright 2017 Scott Leland Crossen

#include "fdqueue.h"

void queue_fd(int connfd) {
  if (fd_queue != NULL) {
    // Tell the compiler that we're smarter than it.
    enqueueLinked(fd_queue->queue, (void*) (long) connfd);
    V(&(fd_queue->entries));
  }
}

int dequeue_fd(int* fd) {
  void* raw_fd;

  if(fd_queue != NULL) {
    P(&(fd_queue->entries));
    if(dequeueLinked(fd_queue->queue, &raw_fd) != -1) {
      *fd = (int) (long) raw_fd;
      return 0;
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}

void init_fd(){
  fd_queue = malloc(sizeof(fd_queue_t));
  fd_queue->queue = createLinkedQueue(REQ_QUEUE_INIT_SIZE, FULL_RESIZE, EMPTY_IGNORE);
  Sem_init(&fd_queue->entries, 0, 0);
}
