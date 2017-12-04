#include "csapp.h"

// Defined in class handout
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

// An object stored in cache. Private. A pointer is returned to data in methods.
typedef struct cache_object {
    char* id;
    void* data;
    struct cache_object* next;
    unsigned int length;
} cache_object;

// The data structure containing the cache for the proxy
typedef struct cache_root {
    cache_object* head;
    cache_object* rear;
    sem_t read;
    sem_t write;
    unsigned remaining_length;
    unsigned int read_count;
} cache_root;

// Methods available for modifying the cache
cache_root* init_cache();
int add_data_to_cache(cache_root* queue, char* id, void* content, unsigned int length);
int read_data_from_cache(cache_root* queue, char* id, void* content, unsigned int* length);
