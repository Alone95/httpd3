//
// Created by root on 3/17/16.
//

#include "handle.h"
#include <linux/tcp.h>

int set_nonblock(int file_dsption){
    int old_flg;
    old_flg = fcntl(file_dsption, F_GETFL);
    fcntl(file_dsption, F_SETFL, old_flg | O_NONBLOCK);
    return old_flg;
}

void optimizes(int file_dsption) {
    const int on = 1;
    if(0 != setsockopt(file_dsption, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) ) {
        perror("REUSEADDR: ");
    }
    if(setsockopt(file_dsption, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)))
        perror("TCP_NODELAY: ");

}

/* open_listenfd is aim to make a prepare for listen socket.
 *
 * */
int open_listenfd(const char * host_addr, const char * port, int* sock_type){
    int listenfd = 0; /* listen the Port, To accept the new Connection */
    struct addrinfo info_of_host;
    struct addrinfo * result;
    struct addrinfo * p;

    memset(&info_of_host, 0, sizeof(info_of_host));
    info_of_host.ai_family = AF_UNSPEC; /* Unknown Socket Type */
    info_of_host.ai_flags = AI_PASSIVE; /* Let the Program to help us fill the Message we need */
    info_of_host.ai_socktype = SOCK_STREAM; /* TCP */

    int error_code;
    if(0 != (error_code = getaddrinfo(host_addr, port, &info_of_host, &result))){
        fputs(gai_strerror(error_code), stderr);
        return ERR_GETADDRINFO; /* -2 */
    }

    for(p = result; p != NULL; p = p->ai_next) {
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if(-1 == listenfd)
            continue; /* Try the Next Possibility */
        if(-1 == bind(listenfd, p->ai_addr, p->ai_addrlen)){
            close(listenfd);
            continue; /* Same Reason */
        }
        break; /* If we get here, it means that we have succeed to do all the Work */
    }
    if (NULL == p) {
        /* TODO ERROR HANDLE */
        fprintf(stderr, "In %s, Line: %d\nError Occur while Open/Binding the listen fd\n",__FILE__, __LINE__);
        return ERR_BINDIND;
    }
    
    fprintf(stderr, "DEBUG MESG: Now We(%d) are in : %s , listen the %s port Success\n", listenfd,
            inet_ntoa(((struct sockaddr_in *)p->ai_addr)->sin_addr), port);
    *sock_type = p->ai_family;
    freeaddrinfo(result);
    set_nonblock(listenfd);
    optimizes(listenfd);
    return listenfd;
}

