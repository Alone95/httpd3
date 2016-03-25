//
// Created by root on 3/21/16.
//

#ifndef HTTPD3_HTTP_RESPONSE_H
#define HTTPD3_HTTP_RESPONSE_H
#include "handle_core.h"
typedef enum {
    MAKE_PAGE_SUCCESS = 0,
    MAKE_PAGE_FAIL = 1 << 0,
}MAKE_PAGE_STATUS;

MAKE_PAGE_STATUS make_response_page(conn_client * client,
                                    const char * http_ver, const char * uri, const char * method);

#endif //HTTPD3_HTTP_RESPONSE_H
