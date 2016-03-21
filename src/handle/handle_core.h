//
// Created by root on 3/20/16.
//

#ifndef HTTPD3_HANDLE_CORE_H
#define HTTPD3_HANDLE_CORE_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
struct connection {
    int  linger;
    int  epfd_grop;
    int  file_dsp;
#define BUF_SIZE 8192
    int  read_offset;
    char read_buf[BUF_SIZE];
    int  write_offset;
    char write_buf[BUF_SIZE];
};
typedef struct connection conn_client;

typedef enum {
    HANDLE_READ_SUCCESS = -(1 << 1),
    HANDLE_READ_FAILURE = -(1 << 2),
    HANDLE_WRITE_SUCCESS = -(1 << 3),
    HANDLE_WRITE_FAILURE = -(1 << 4),
}HANDLE_STATUS;

#endif //HTTPD3_HANDLE_CORE_H
