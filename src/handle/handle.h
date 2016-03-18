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
int open_listenfd(const char * host_addr,const char * port, int* sock_type);

/*
 * Let the file description to be Nonblock
 * */
int set_nonblock(int file_dsption);

/*
 * Set the SO_REUSEADDR and TCP_NODELAY
 */
void optimizes(int file_dsption);


#endif //HTTPD3_HANDLE_H
