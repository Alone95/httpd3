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

/*
 * Make a response page by the giving Message
 * @param  client   : connection's message
 *         http_ver : what kind of http will be used.
 *         uri      : the Resource that will be sent.
 *         method   : which method(GET or POST or so on...)
 * @return Means that does the page make or send success?
 * */
MAKE_PAGE_STATUS make_response_page(conn_client * client,
                                    const char * http_ver, const char * uri, const char * method);

#endif //HTTPD3_HTTP_RESPONSE_H
