//
// Created by root on 3/20/16.
//
#include <string.h>
#include <assert.h>
#include "handle_read.h"
#include "http_response.h"

#define READ_LINE 2048
static int read_n(conn_client * client);

HANDLE_STATUS handle_read(conn_client * client) {
    int err_code = 0;

    /* Reading From Socket */
    err_code = read_n(client);
    if (err_code != READ_SUCCESS) { /* If read Fail then End this connect */
        return HANDLE_READ_FAILURE;
    }
#if defined(WSX_DEBUG)
    fprintf(stderr, "\nRead From Client(%d): %s\n", client->file_dsp, client->read_buf);
#endif
    /* Parsing the Reading Data */
#if defined(WSX_DEBUG)
    fprintf(stderr, "\nStart Parsing Reading\n");
#endif
    err_code = parse_reading(client);
    if(err_code != PARSE_SUCCESS) {
        return HANDLE_READ_FAILURE;
    }
    return HANDLE_READ_SUCCESS;
}

static int read_n(conn_client * client) {
    if (client->read_offset >= BUF_SIZE-1)
        return READ_OVERFLOW;
    int    fd        = client->file_dsp;
    char * buf       = client->read_buf;
    int    buf_index = client->read_offset;
    int read_number = 0;
    while (1) {
        read_number = read(fd, buf+buf_index, BUF_SIZE-buf_index);
        if (0 == read_number) { /* We must close connection */
            return READ_FAIL;
        }
        else if (-1 == read_number) { /* Nothing to read */
            if (EAGAIN == errno || EWOULDBLOCK == errno) {
                buf[buf_index] = '\0';
                return READ_SUCCESS;
            }
            return READ_FAIL;
        }
        else {
            buf_index += read_number;
            client->read_offset = buf_index;
        }
    }
}

/******************************************************************************************/
/* Deal with the read buf data which has been read from socket */

static int get_line(conn_client * restrict client, char * restrict line_buf, int max_line);
static DEAL_LINE_STATUS deal_requ(conn_client * client);
static DEAL_LINE_STATUS deal_head(conn_client * client);
/*
 * Now we believe that the GET has no request-content(Request Line && Request Head)
 * The POST Will be considered later
 * parse_reading will parse the request line and Request Head,
 * and Prepare the Page which will set into the Write buffer
 * */
PARSE_STATUS parse_reading(conn_client * client) {
    int err_code = 0;
    client->read_offset = 0; /* Set the offset to 0, the end of buf is '\0' */
    /*  Request line */
    err_code = deal_requ(client);
    if (DEAL_LINE_REQU_FAIL == err_code)
        return PARSE_BAD_REQUT;
    /* Request Head Attribute until /r/n */
#if defined(WSX_DEBUG)
    fprintf(stderr, "Starting Deal_head\n");
#endif
    err_code = deal_head(client);
    if (DEAL_HEAD_FAIL == err_code)
        return PARSE_BAD_SYNTAX;
    return PARSE_SUCCESS;
}

#define METHOD_SIZE 128
#define PATH_SIZE METHOD_SIZE*2
#define VERSION_SIZE METHOD_SIZE
/*
 * deal_requ
 * */
static DEAL_LINE_STATUS deal_requ(conn_client * client) {
    char requ_line[READ_LINE] = {'\0'};
    int err_code = get_line(client, requ_line, READ_LINE);
#if defined(WSX_DEBUG)
    assert(err_code > 0);
#endif
    if (err_code < 0)
        return DEAL_LINE_REQU_FAIL;
#if defined(WSX_DEBUG)
    fprintf(stderr, "Requset line is : %s \n", requ_line);
#endif
    /* For example GET / HTTP/1.0 */
    char method[METHOD_SIZE]   = {'\0'};
    char path[PATH_SIZE]       = {'\0'};
    char version[VERSION_SIZE] = {'\0'};
    sscanf(requ_line, "%s %s %s", method, path, version);
    fprintf(stderr, "The Request method : %s, path : %s, version : %s\n", method, path, version);
    /* Test the Method. Did it implemented by Server */
    if (MAKE_PAGE_FAIL == make_response_page(client, version, path, method))
        return DEAL_LINE_REQU_FAIL;
    return DEAL_LINE_REQU_SUCCESS;
}
static DEAL_LINE_STATUS deal_head(conn_client * client) {
    int nbytes = 0;
    char head_line[READ_LINE] = {'\0'};
    int index = 1;
    while((nbytes = get_line(client, head_line, READ_LINE)) > 0) {
        if(0 == strncmp(head_line, "\r\n", 2)) {
            fprintf(stderr, "Read the empty Line\n");
            break;
        }
#if defined(WSX_DEBUG)
        fprintf(stderr, "The %d Line we parse(%d Bytes) : ", index++, nbytes);
        fprintf(stderr, "%s", head_line);
#endif
    }
    if (nbytes < 0) {
        fprintf(stderr, "Error Reading in deal_head\n");
        return DEAL_HEAD_FAIL;
    }
    fprintf(stderr, "Deal head Success\n");
    return DEAL_HEAD_SUCCESS;
}
static int get_line(conn_client * restrict client, char * restrict line_buf, int max_line) {
    int read_idx = client->read_offset;
    const char * read_ptr = client->read_buf;
    while(read_ptr[read_idx] != '\n' && read_ptr[read_idx] != '\0')
        ++read_idx;
    char c = read_ptr[read_idx++]; /* ++ means that go into next start character */
    int nbytes = 0;
    if ( '\0' == c ) { /* if get the '\0' */
        fprintf(stderr, "get_line has read a 0\n");
        return READ_BUF_FAIL;
    }
    else if ('\n' == c) {
        nbytes = read_idx - client->read_offset;
        if (max_line-1 >=  nbytes) {
            memcpy(line_buf, read_ptr + client->read_offset, nbytes);
            client->read_offset = read_idx;
        }
        else {
            fprintf(stderr, "get_line has overflow\n");
            return READ_BUF_OVERFLOW;
        }
    }
    line_buf[nbytes] = '\0';
    return nbytes;
}
