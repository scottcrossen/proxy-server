#include "cache.h"

cache_object* delete_cache_object_from_head(cache_root* queue);
cache_object* delete_cache_object(cache_root* queue, char* id);
cache_object* init_cache_object(char* id, int length);
cache_object* search_for_cache_object(cache_root* queue, char* id);
void add_cache_object_to_rear_sync(cache_root* queue, cache_object* node);
void add_cache_object_to_rear(cache_root* queue, cache_object* node);
void destory_cache_object(cache_object* element);
void evict_lru_cache_object(cache_root* queue);

cache_root* init_cache() {
  cache_root* queue = (cache_root*) malloc(sizeof(cache_root));

  queue->head = NULL;
  queue->rear = NULL;
  queue->remaining_length = MAX_CACHE_SIZE;
  Sem_init(&queue->read, 0, 1);
  Sem_init(&queue->write, 0, 1);
  queue->read_count = 0;
  return queue;
}

int read_data_from_cache(
  cache_root* queue,
  char* id,
  void* data,
  unsigned int* length
) {
  if (queue != NULL) {
    P(&(queue->read));
    queue->read_count++;
    if (queue->read_count == 1) {
      P(&(queue->write));
    }
    V(&(queue->read));
    cache_object* node = search_for_cache_object(queue, id);
    if (node == NULL) {
      P(&(queue->read));
      queue->read_count--;
      if (queue->read_count == 0) {
        V(&(queue->write));
      }
      V(&(queue->read));
      return -1;
    } else {
      *length = node->length;
      memcpy(data, node->data,* length);
      P(&(queue->read));
      queue->read_count--;
      if (queue->read_count == 0) {
        V(&(queue->write));
      }
      V(&(queue->read));
      P(&(queue->write));
      node = delete_cache_object(queue, id);
      add_cache_object_to_rear(queue, node);
      V(&(queue->write));
      return 0;
    }
  } else {
    return -1;
  }
}

int add_data_to_cache(
  cache_root* queue,
  char* id,
  void* data,
  unsigned int length
) {
  if (queue != NULL) {
    cache_object* node = init_cache_object(id, length);
    if (node != NULL) {
      memcpy(node->data, data, length);
      node->length = length;
      add_cache_object_to_rear_sync(queue, node);
      return 0;
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}

cache_object* init_cache_object(
  char* id,
  int length
) {
  cache_object* node = (cache_object*) malloc(sizeof(cache_object));

  if (node != NULL) {
    node->id = (char*) Malloc(sizeof(char)*  (strlen(id) + 1));
    if (node->id != NULL) {
      strcpy(node->id, id);
      node->length = 0;
      node->data = Malloc(length);
      if (node->data != NULL) {
        node->next = NULL;
        return node;
      } else {
        return NULL;
      }
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

cache_object* search_for_cache_object(
  cache_root* list,
  char* id
) {
  if (list != NULL) {
    cache_object* cur_node = list->head;
    while (cur_node != NULL) {
      if (strcmp(cur_node->id, id) == 0) {
        return cur_node;
      }
      cur_node = cur_node->next;
    }
    return NULL;
  } else {
    return NULL;
  }
}

void add_cache_object_to_rear(
  cache_root* list,
  cache_object* node
) {
  if (list != NULL) {
    if (list->head == NULL) {
      list->head = list->rear = node;
      list->remaining_length -= node->length;
    } else {
      list->rear->next = node;
      list->rear = node;
      list->remaining_length -= node->length;
    }
  }
}

void add_cache_object_to_rear_sync(
  cache_root* queue,
  cache_object* element
) {
  if (queue != NULL) {
    P(&(queue->write));
    while (queue->remaining_length < element->length) {
      evict_lru_cache_object(queue);
    }
    add_cache_object_to_rear(queue, element);
    V(&(queue->write));
  }
}

cache_object* delete_cache_object_from_head(cache_root* queue) {
  if (queue != NULL) {
    cache_object* cur_node = queue->head;
    if (cur_node != NULL) {
      queue->head = cur_node->next;
      queue->remaining_length += cur_node->length;
      if (cur_node == queue->rear) {
        queue->rear = NULL;
      }
      return cur_node;
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

void evict_lru_cache_object(cache_root* queue) {
  if (queue != NULL) {
    cache_object* node = delete_cache_object_from_head(queue);
    Free(node->id);
    Free(node->data);
    Free(node);
  }
}

cache_object* delete_cache_object(
  cache_root* queue,
  char* id
) {
  if (queue != NULL) {
    cache_object* prev_node = NULL;
    cache_object* cur_node = queue->head;

    while (cur_node != NULL) {
      if (strcmp(cur_node->id, id) == 0) {
        if (queue->head == cur_node) {
          queue->head = cur_node->next;
        }
        if (queue->rear == cur_node) {
          queue->rear = prev_node;
        }
        if (prev_node != NULL) {
          prev_node->next = cur_node->next;
        }
        cur_node->next = NULL;
        queue->remaining_length += cur_node->length;
        return cur_node;
      } else {
        prev_node = cur_node;
        cur_node = cur_node->next;
      }
    }
    return NULL;
  } else {
    return NULL;
  }
}
