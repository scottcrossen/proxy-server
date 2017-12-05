// @Copyright 2017 Scott Leland Crossen

#include "squeue.h"
#include "csapp.h"

#pragma once

#define LOG_MAX_LENGTH 255

#define LOG_QUEUE_INIT_SIZE 16

#define log(message) \
  queue_log(message)

#define logf(message, ...) \
  char _logf_temp[LOG_MAX_LENGTH] = {0}; \
  snprintf(_logf_temp, LOG_MAX_LENGTH, message "\r\n", ## __VA_ARGS__); \
  log(_logf_temp)

void queue_log(char* message);

linked_queue_t* log_queue;

void init_logger();

void* logger_routine(void* vargp);
