//
// Created by root on 3/17/16.
//

#ifndef HTTPD3_HANDLE_H
#define HTTPD3_HANDLE_H

#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

/* For wsx_config_t */
#include "../read_config.h"
/* For memory allocation */
#include "../memop/manage.h"
#define MAX_LISTEN_EPFD_SIZE 1
#define OPEN_FILE 100000

struct connection {
    int epfd_grop;
    int file_dsp;
    int read_offset;
    char * read_buf;
    int write_offset;
    char * write_buf;
};
typedef struct connection conn_client;

enum { ERR_PARA_EMPTY = -1,
    ERR_GETADDRINFO = -2,
    ERR_BINDIND = -3};

/*
 * Open The Listen Socket With the specific host(IP address) and port
 * That must be compatible with the IPv6 And IPv4
 * host_addr could be NULL
 * port MUST NOT BE NULL !!!
 * sock_type is the pointer to a memory ,which com from the Outside(The Caller)
 * */
int open_listenfd(const char * restrict host_addr,const char * restrict port, int * restrict sock_type);

/*
 * Let the file description to be Nonblock
 * */
int set_nonblock(int file_dsption);

/*
 * Set the SO_REUSEADDR and TCP_NODELAY
 */
void optimizes(int file_dsption);

/* Called in main function
 * Main Structure, create Workers' thread and listen's thread
 * */
void handle_loop(int file_dsption, int sock_type, const wsx_config_t * config);
#endif //HTTPD3_HANDLE_H
