// @Copyright 2017 Scott Leland Crossen

#include "squeue.h"
#include "csapp.h"

#pragma once

#define LOG_QUEUE_INIT_SIZE 16

#define log(message) \
  enqueueLinked(log_queue, (void*) message)

linked_queue_t* log_queue;

void init_logger();

void* logger_routine(void* vargp);
