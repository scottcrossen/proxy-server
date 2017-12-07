// @Copyright 2017 Scott Leland Crossen

#include "logger.h"

void current_time(char buffer[26]);

void init_logger() {
  // Create global and spawn thread
  pthread_t tid;

  log_queue = malloc(sizeof(log_queue_t));
  log_queue->queue = createLinkedQueue(LOG_QUEUE_INIT_SIZE, FULL_RESIZE, EMPTY_WAIT);
  Sem_init(&log_queue->entries, 0, 0);
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
  if (log_queue != NULL) {
    while(1) {
      P(&(log_queue->entries));
      if(dequeueLinked(log_queue->queue, &raw_message) != -1) {
        // Message found
        char* message = ((char*) raw_message);
        current_time(time);
        printf("%s\t%s\n", time, message); fflush(stdout);
        free(message);
      }
    }
  }
  Pthread_exit(NULL);
  return NULL;
}

void queue_log(char* message) {
  if (log_queue != NULL) {
    // Make a new pointer to save the message.
    char* stored_message = (char*) malloc(sizeof(char) * (strlen(message) + 1));
    strcpy(stored_message, message);
    enqueueLinked(log_queue->queue, (void*) stored_message);
    V(&(log_queue->entries));
  }
}
