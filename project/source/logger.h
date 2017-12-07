// @Copyright 2017 Scott Leland Crossen

#include "squeue.h"
#include "csapp.h"

#pragma once

#define LOG_MAX_LENGTH 255

#define LOG_QUEUE_INIT_SIZE 15

#define log(message) \
  queue_log(message)

#define logf(message, ...) \
  char _logf_temp[LOG_MAX_LENGTH] = {0}; \
  snprintf(_logf_temp, LOG_MAX_LENGTH, message "\r\n", ## __VA_ARGS__); \
  log(_logf_temp)

typedef struct log_queue_t {
  linked_queue_t* queue;
  // Not sure if the queue wait behavior works so I'll just use a semaphore here
  sem_t           entries;
} log_queue_t;

log_queue_t* log_queue;

void queue_log(char* message);

void init_logger();

void* logger_routine(void* vargp);
