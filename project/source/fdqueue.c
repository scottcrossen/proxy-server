// @Copyright 2017 Scott Leland Crossen

#include "fdqueue.h"

typedef linked_queue_t fdqueue_t;

void queue_fd(fdqueue_t* fdqueue, int connfd) {
  // Tell the compiler that we're smarter than it.
  enqueueLinked(fdqueue, (void*) (long) connfd);
}

int dequeue_fd(fdqueue_t* fdqueue, int* fd) {
  void* raw_fd;

  if(dequeueLinked(fdqueue, &raw_fd) != -1) {
    *fd = (int) (long) raw_fd;
    return 0;
  } else {
    return -1;
  }
}

fdqueue_t* init_fdqueue(){
  return createLinkedQueue(REQ_QUEUE_INIT_SIZE, FULL_RESIZE, EMPTY_IGNORE);
}
