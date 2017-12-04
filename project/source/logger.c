// @Copyright 2017 Scott Leland Crossen

#include "logger.h"

void current_time(char buffer[26]);

void init_logger() {
  // Create global and spawn thread
  pthread_t tid;
  log_queue = createLinkedQueue(LOG_QUEUE_INIT_SIZE, FULL_RESIZE, EMPTY_IGNORE);
  Pthread_create(&tid, NULL, logger_routine, NULL);
}

void current_time(char buffer[26]) {
  time_t timer;
  struct tm* time_info;

  // I found this online. (Stack Exchange / Overflow)
  time(&timer);
  time_info = localtime(&timer);
  strftime(buffer, 26, "%Y-%m-%dT%H:%M:%SZ", time_info);
}

void* logger_routine(void* vargp) {
  void* raw_message;
  char time[26];

  // Run in detached mode
  Pthread_detach(pthread_self());
  while(1) {
    if(log_queue != NULL && dequeueLinked(log_queue, &raw_message) != -1) {
      // Message found
      char* message = ((char*) raw_message);
      current_time(time);
      printf("%s\t%s\n", time, message); fflush(stdout);
    }
  }
  Pthread_exit(NULL);
  return NULL;
}
