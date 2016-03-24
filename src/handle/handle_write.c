//
// Created by root on 3/22/16.
//
#include "handle_write.h"

HANDLE_STATUS handle_write(conn_client * client) {
    int nbyte = client->write_offset;
    int count = 0;
    int fd = client->file_dsp;
    char * buf = client->write_buf;

    while (nbyte > 0) {
        buf += count;
        count = write(fd, buf, 8192);
        if (count < 0) {
            if (EAGAIN == errno || EWOULDBLOCK == errno) {
                memcpy(client->write_buf, buf, strlen(buf));
                client->write_offset = nbyte;
                return HANDLE_WRITE_AGAIN;
            }
            if (EPIPE == errno)
                return HANDLE_WRITE_FAILURE;
        }
        else if (0 == count)
            return HANDLE_WRITE_FAILURE;
        nbyte -= count;
    }
    return HANDLE_WRITE_SUCCESS;
}


