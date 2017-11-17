/*
 TODO: add comments,
 TODO: remove cache
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "cache.h"

static const char* client_bad_request = "HTTP/1.1 400 Bad Request\r\nServer: Apache\r\nContent-Length: 140\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<html><head></head><body><p>Bad Request</p></body></html>";
static const char* connection_close = "Connection: close\r\n";
static const char* http_version = "HTTP/1.0\r\n";
static const char* proxy_connection_close = "Proxy-Connection: close\r\n";
static const char* user_agent = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct sockaddr_in sockaddr_in;
typedef struct thread_args { cache_queue* cache; int connfd; } thread_args;

int cache_and_serve(char* buffer, int to_client_fd, int* valid_obj_size, void* cache_content, unsigned int* cache_length, unsigned int length);
int process_non_get_request(char* buffer, rio_t rio_client, char* host_port, int* to_server_fd);
int parse_request(char* buffer, char* method, char* protocol, char* host_port, char* resource, char* version);
int process_get_request(cache_queue* cache, int fd, char* buffer, char* method, char* resource, rio_t rio_client, char* host_port, int* to_server_fd, char* cache_id, void* cache_content, unsigned int* cache_length);
int request_from_server(cache_queue* cache, int fd, int* to_server_fd, char* cache_id, void* cache_content, unsigned int* cache_length);
int serve_client_and_cache(cache_queue* cache, int to_client_fd, int to_server_fd, char* cache_id, void* cache_content);
int serve_client_from_cache(int to_client_fd, void* cache_content, unsigned int cache_length);
int serve_client(int to_client_fd, int to_server_fd);
void close_connection(int* to_client_fd, int* to_server_fd);
void parse_host(char* host_port, char* remote_host, char* remote_port);
void* thread_routine(void* arg);

#define READ_FROM_CACHE 1
#define NON_GET_METHOD 2

int main(int argc, char* argv []) {
  char* port;
  int listenfd;
  pthread_t tid;
  sockaddr_in clientaddr;
  socklen_t clientlen;
  thread_args args;

  Signal(SIGPIPE, SIG_IGN);

  if (argc != 2 ) {
    fprintf(stderr, "Usage: %s [port]", argv[0]);
    exit(1);
  } else if (atoi(argv[1]) <= 0) {
    fprintf(stderr, "Port number invalid\n");
    exit(1);
  } else {
    port = argv[1];
    if ((listenfd = Open_listenfd(port)) < 0) {
      fprintf(stderr, "Error attaching to port %d\n", *port);
      exit(1);
    } else {
      args.cache = init_cache();
      while (1) {
        clientlen = sizeof(clientaddr);
        args.connfd = Accept(listenfd, (SA*) &clientaddr, (socklen_t*) &clientlen);
        Pthread_create(&tid, NULL, thread_routine, &args);
      }
    }
  }
  return 0;
}

void* thread_routine(void* vargp) {
  thread_args* args = (thread_args*) vargp;
  int client_fd = (args->connfd);
  cache_queue* cache = (args->cache);
  char cache_content[MAX_OBJECT_SIZE];
  char cache_id[MAXLINE];
  int ret_val = 0;
  int server_fd = -1;
  unsigned int cache_length;

  Pthread_detach(pthread_self());

  if ((ret_val = request_from_server(cache, client_fd, &server_fd, cache_id, cache_content, &cache_length)) == -1) {
    close_connection(&client_fd, &server_fd);
    Pthread_exit(NULL);
  } else if (ret_val == READ_FROM_CACHE) {
    if (serve_client_from_cache(client_fd, cache_content, cache_length) == -1) {
      close_connection(&client_fd, &server_fd);
      Pthread_exit(NULL);
    }
  } else if (ret_val == NON_GET_METHOD) {
    if (serve_client(client_fd, server_fd) == -1) {
      close_connection(&client_fd, &server_fd);
      Pthread_exit(NULL);
    }
  } else {
    if (serve_client_and_cache(cache, client_fd, server_fd, cache_id, cache_content) == -1) {
      close_connection(&client_fd, &server_fd);
      Pthread_exit(NULL);
    }
  }
  close_connection(&client_fd, &server_fd);
  return NULL;
}

int process_get_request(
  cache_queue* cache,
  int fd,
  char* buffer,
  char* method,
  char* resource,
  rio_t rio_client,
  char* host_port,
  int* to_server_fd,
  char* cache_id,
  void* cache_content,
  unsigned int* cache_length
) {
  char origin_host_header[MAXLINE];
  char remote_host[MAXLINE];
  char remote_port[MAXLINE];
  char request_buffer[MAXLINE];

  strcpy(remote_host, "");
  strcpy(remote_port, "80");
  parse_host(host_port, remote_host, remote_port);
  strcpy(request_buffer, method);
  strcat(request_buffer, " ");
  strcat(request_buffer, resource);
  strcat(request_buffer, " ");
  strcat(request_buffer, http_version);
  while (Rio_readlineb(&rio_client, buffer, MAXLINE) != 0) {
    if (strcmp(buffer, "\r\n") == 0) {
      break;
    } else if (strstr(buffer, "User-Agent:") != NULL) {
      strcat(request_buffer, user_agent);
    } else if (strstr(buffer, "Connection:") != NULL) {
      strcat(request_buffer, connection_close);
    } else if (strstr(buffer, "Proxy Connection:") != NULL) {
      strcat(request_buffer, proxy_connection_close);
    } else if (strstr(buffer, "Host:") != NULL) {
      strcpy(origin_host_header, buffer);
      if (strlen(remote_host) < 1) {
        sscanf(buffer, "Host: %s", host_port);
        parse_host(host_port, remote_host, remote_port);
      }
      strcat(request_buffer, buffer);
    } else {
      strcat(request_buffer, buffer);
    }
  }
  strcat(request_buffer, "\r\n");
  if (strcmp(remote_host, "") == 0) {
    return -1;
  } else {
    strcpy(cache_id, method);
    strcat(cache_id, " ");
    strcat(cache_id, remote_host);
    strcat(cache_id, ":");
    strcat(cache_id, remote_port);
    strcat(cache_id, resource);
    if (read_cache_element_lru_sync(cache, cache_id, cache_content, cache_length) != -1) {
      return READ_FROM_CACHE;
    } else {
      if ((*to_server_fd = Open_clientfd(remote_host, remote_port)) == -1) {
        return -1;
      } else if (*to_server_fd == -2) {
        strcpy(buffer, client_bad_request);
        Rio_writen(fd, buffer, strlen(buffer));
        return -1;
      } else if (Rio_writen(*to_server_fd, request_buffer, strlen(request_buffer)) == -1) {
        return -1;
      } else {
        return 0;
      }
    }
  }
}

int process_non_get_request(
  char* buffer,
  rio_t rio_client,
  char* host_port,
  int* to_server_fd
) {
  char origin_host_header[MAXLINE];
  char remote_host[MAXLINE];
  char remote_port[MAXLINE];
  char request_buffer[MAXLINE];
  unsigned int length = 0;
  unsigned int size = 0;

  strcpy(remote_host, "");
  strcpy(remote_port, "80");
  parse_host(host_port, remote_host, remote_port);
  strcpy(request_buffer, buffer);
  while (strcmp(buffer, "\r\n") != 0 && strlen(buffer) > 0) {
    if (Rio_readlineb(&rio_client, buffer, MAXLINE) == -1) {
      return -1;
    } else {
      if (strstr(buffer, "Host:") != NULL) {
        strcpy(origin_host_header, buffer);
        if (strlen(remote_host) < 1) {
          sscanf(buffer, "Host: %s", host_port);
          parse_host(host_port, remote_host, remote_port);
        }
      }
      if (strstr(buffer, "Content-Length")) {
        sscanf(buffer, "Content-Length: %d", &size);
      }
      strcat(request_buffer, buffer);
    }
  }
  if (strcmp(remote_host, "") == 0) {
    return -1;
  } else {
    char port = atoi(remote_port);
    if ((*to_server_fd = Open_clientfd(remote_host, &port)) < 0) {
      return -1;
    } else if (Rio_writen(*to_server_fd, request_buffer, strlen(request_buffer)) == -1) {
      return -1;
    } else {
      while (size > MAXLINE) {
        if ((length = Rio_readnb(&rio_client, buffer, MAXLINE)) == -1) {
          return -1;
        } else if (Rio_writen(*to_server_fd, buffer, length) == -1) {
          return -1;
        } else {
          size -= MAXLINE;
        }
      }
      if (size > 0) {
        if ((length = Rio_readnb(&rio_client, buffer, size)) == -1) {
          return -1;
        } else if (Rio_writen(*to_server_fd, buffer, length) == -1) {
          return -1;
        }
      }
      return NON_GET_METHOD;
    }
  }
}

int request_from_server(
  cache_queue* cache,
  int fd,
  int* to_server_fd,
  char* cache_id,
  void* cache_content,
  unsigned int* cache_length
) {
  char buffer[MAXLINE];
  char host_port[MAXLINE];
  char method[MAXLINE];
  char resource[MAXLINE];
  rio_t rio_client;

  memset(cache_content, 0, MAX_OBJECT_SIZE);
  Rio_readinitb(&rio_client, fd);
  if (Rio_readlineb(&rio_client, buffer, MAXLINE) == -1) {
    printf("%s\n", buffer);
    return -1;
  } else {
    char protocol[MAXLINE];
    char version[MAXLINE];
    if (parse_request(buffer, method, protocol, host_port, resource, version) == -1) {
      return -1;
    } else {
      if (strstr(method, "GET") != NULL) {
        return process_get_request(cache, fd, buffer, method, resource, rio_client, host_port, to_server_fd, cache_id, cache_content, cache_length);
      } else {
        return process_non_get_request(buffer, rio_client, host_port, to_server_fd);
      }
    }
  }
}

int serve_client(int to_client_fd, int to_server_fd) {
  char buffer[MAXLINE];
  rio_t rio_server;
  unsigned int length = 0;
  unsigned int size = 0;

  Rio_readinitb(&rio_server, to_server_fd);
  if (Rio_readlineb(&rio_server, buffer, MAXLINE) == -1) {
    return -1;
  } else if (Rio_writen(to_client_fd, buffer, strlen(buffer)) == -1) {
    return -1;
  } else {
    while (strcmp(buffer, "\r\n") != 0 && strlen(buffer) > 0) {
      if (Rio_readlineb(&rio_server, buffer, MAXLINE) == -1) {
        return -1;
      } else {
        if (strstr(buffer, "Content-Length")) {
          sscanf(buffer, "Content-Length: %d", &size);
        }
        if (Rio_writen(to_client_fd, buffer, strlen(buffer)) == -1) {
          return -1;
        }
      }
    }
    if (size > 0) {
      while (size > MAXLINE) {
        if ((length = Rio_readnb(&rio_server, buffer, MAXLINE)) == -1) {
          return -1;
        } else if (Rio_writen(to_client_fd, buffer, length) == -1) {
          return -1;
        } else {
          size -= MAXLINE;
        }
      }
      if (size > 0) {
        if ((length = Rio_readnb(&rio_server, buffer, size)) == -1) {
          return -1;
        } else if (Rio_writen(to_client_fd, buffer, length) == -1) {
          return -1;
        }
      }
    } else {
      while ((length = Rio_readnb(&rio_server, buffer, MAXLINE)) > 0) {
        if (Rio_writen(to_client_fd, buffer, length) == -1) {
          return -1;
        }
      }
    }
    return 0;
  }
}

int serve_client_from_cache(
  int to_client_fd,
  void* cache_content,
  unsigned int cache_length
) {
  if (Rio_writen(to_client_fd, cache_content, cache_length)) {
    return -1;
  } else {
    return 0;
  }
}

int cache_and_serve(
  char* buffer,
  int to_client_fd,
  int* valid_obj_size,
  void* cache_content,
  unsigned int* cache_length,
  unsigned int length
) {
  void* current_cache_position;

  if (*valid_obj_size) {
    if ((*cache_length + strlen(buffer)) > MAX_OBJECT_SIZE) {
      *valid_obj_size = 0;
    } else {
      current_cache_position = (void*) ((char*) cache_content + *cache_length);
      memcpy(current_cache_position, buffer, strlen(buffer));
      *cache_length = *cache_length + strlen(buffer);
      *valid_obj_size = 1;
    }
  }
  if (Rio_writen(to_client_fd, buffer, length) == -1) {
    return -1;
  } else {
    return 0;
  }
}

int serve_client_and_cache(
  cache_queue* cache,
  int to_client_fd,
  int to_server_fd,
  char* cache_id,
  void* cache_content
) {
  char buffer[MAXLINE];
  int valid_obj_size = 1;
  rio_t rio_server;
  unsigned int cache_length = 0;
  unsigned int length = 0;
  unsigned int size = 0;

  Rio_readinitb(&rio_server, to_server_fd);
  if (Rio_readlineb(&rio_server, buffer, MAXLINE) == -1) {
    return -1;
  } else {
    if(cache_and_serve(buffer, to_client_fd, &valid_obj_size, cache_content, &cache_length, strlen(buffer)) == -1) {
      return -1;
    } else {
      while (strcmp(buffer, "\r\n") != 0 && strlen(buffer) > 0) {
        if (Rio_readlineb(&rio_server, buffer, MAXLINE) == -1) {
          return -1;
        } else {
          if (strstr(buffer, "Content-Length")) {
            sscanf(buffer, "Content-Length: %d", &size);
          }
          if(cache_and_serve(buffer, to_client_fd, &valid_obj_size, cache_content, &cache_length, strlen(buffer)) == -1) {
            return -1;
          }
        }
      }
      if (size > 0) {
        while (size > MAXLINE) {
          if ((length = Rio_readnb(&rio_server, buffer, MAXLINE)) == -1) {
            return -1;
          } else {
            if(cache_and_serve(buffer, to_client_fd, &valid_obj_size, cache_content, &cache_length, length) == -1) {
              return -1;
            } else {
              size -= MAXLINE;
            }
          }
        }
        if (size > 0) {
          if ((length = Rio_readnb(&rio_server, buffer, size)) == -1) {
            return -1;
          } else if (cache_and_serve(buffer, to_client_fd, &valid_obj_size, cache_content, &cache_length, length) == -1) {
            return -1;
          }
        }
      } else {
        while ((length = Rio_readnb(&rio_server, buffer, MAXLINE)) > 0) {
          if(cache_and_serve(buffer, to_client_fd, &valid_obj_size, cache_content, &cache_length, length) == -1) {
            return -1;
          }
        }
      }
      if (valid_obj_size && (add_data_to_cache_sync(cache, cache_id, cache_content, cache_length) == -1)) {
        return -1;
      } else {
        return 0;
      }
    }
  }
}

int parse_request (
  char* buffer,
  char* method,
  char* protocol,
  char* host_port,
  char* resource,
  char* version
) {
  char url[MAXLINE];

  if (strstr(buffer, "/") == NULL || strlen(buffer) < 1) {
    return -1;
  } else {
    strcpy(resource, "/");
    sscanf(buffer, "%s %s %s", method, url, version);
    if (strstr(url, "://") != NULL) {
      sscanf(url, "%[^:]://%[^/]%s", protocol, host_port, resource);
    } else {
      sscanf(url, "%[^/]%s", host_port, resource);
    }
    return 0;
  }
}

void parse_host(char* host_port, char* remote_host, char* remote_port) {
  char* tmp = NULL;

  tmp = index(host_port, ':');
  if (tmp != NULL) {
    *tmp = '\0';
    strcpy(remote_port, tmp + 1);
  } else {
    strcpy(remote_port, "80");
  }
  strcpy(remote_host, host_port);
}

void close_connection(int* to_client_fd, int* to_server_fd) {
  if (*to_client_fd >= 0) {
    Close(*to_client_fd);
    to_client_fd = NULL;
  }
  if (*to_server_fd >= 0) {
    Close(*to_server_fd);
    to_server_fd = NULL;
  }
}
