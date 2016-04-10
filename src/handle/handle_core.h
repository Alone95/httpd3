//
// Created by root on 3/20/16.
//

#ifndef HTTPD3_HANDLE_CORE_H
#define HTTPD3_HANDLE_CORE_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "../memop/manage.h"
#include "../util/wsx_string.h"
typedef char boolean;
struct connection {
    int  file_dsp;
#define CONN_BUF_SIZE 1024
    int  read_offset;
    char read_buf[CONN_BUF_SIZE];
    /*int  write_offset;*/
    char write_buf[CONN_BUF_SIZE];
    int r_buf_offset;
    string_t r_buf;
    int w_buf_offset;
    string_t w_buf;

    struct {
        /* Is it Keep-alive in Application Layer */
        boolean conn_linger : 1;
        boolean set_ep_out  : 1;
        /* 2 ^ 4 -> 16 Types */
        int content_type    : 4;
        /* GET, POST, HEAD */
        string_t requ_method;
        /* HTTP/1.0\0 */
        string_t requ_http_ver;
        /* / */
        string_t requ_res_path;
    }conn_res;
};
typedef struct connection conn_client;

typedef enum {
    HANDLE_READ_SUCCESS = -(1 << 1),
    HANDLE_READ_FAILURE = -(1 << 2),
    HANDLE_WRITE_SUCCESS = -(1 << 3),
    HANDLE_WRITE_AGAIN   = -(1 << 5),
    HANDLE_WRITE_FAILURE = -(1 << 4),
}HANDLE_STATUS;

#endif //HTTPD3_HANDLE_CORE_H
