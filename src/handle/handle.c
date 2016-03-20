//
// Created by root on 3/17/16.
//

#include "handle.h"
#include <linux/tcp.h>
#include <pthread.h>

#if defined(WSX_DEBUG)
    #include <assert.h>
#endif
enum SERVE_STATUS {
    CLOSE_SERVE = 1,
    RUNNING_SERVER= 0
};
static enum SERVE_STATUS terminal_server = RUNNING_SERVER;

static int* epfd_group = NULL;  /* Workers' epfd set */
static int epfd_group_size = 0; /* Workers' epfd set size */
static int workers = 0;         /* NUmber of Workers */
static int listeners = MAX_LISTEN_EPFD_SIZE; /* Number of Listenner */
static conn_client * clients;   /* Client set */

/* Add new Socket to the epfd
 * */
static inline void add_event(int epfd, int fd, int event_flag) {
    struct epoll_event event = {0};
    event.data.fd = fd;
    event.events = event_flag | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event)) {
        perror("epoll_ctl ADD: ");
        exit(-1);
    }
}
/*
 * Modify including Re-register Each sock while it woke up by epfd
 * */
static inline void mod_event(int epfd, int fd, int event_flag) {
    struct epoll_event event = {0};
    event.data.fd = fd;
    event.events |= EPOLLONESHOT | event_flag;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event)) {
        perror("epoll_ctl MOD: ");
        exit(-1);
    }
}
/*
 * Prepare Some Resources For Workers
 * Worker epfd set and client set
 * */
static inline void prepare_workers(void) {
    epfd_group_size = workers;
    epfd_group = Malloc(epfd_group_size * (sizeof(int)));
#if defined(WSX_DEBUG)
    if (NULL == epfd_group) {
        fputs( "Malloc Fail!", stderr);
        assert(NULL == epfd_group);
    }
#endif
    for (int i = 0; i < epfd_group_size; ++i) {
        epfd_group[i] = epoll_create1(0);
#if defined(WSX_DEBUG)
        if (-1 == epfd_group[i])
            perror("epoll_create1 : ");
        assert(-1 == epfd_group[i]);
#endif
    }
    clients = Malloc(OPEN_FILE * sizeof(conn_client));
#if defined(WSX_DEBUG)
    if (NULL == clients) {
        fputs( "Malloc Fail!", stderr);
        assert(NULL == clients);
    }
#endif
}
/* Listener's Thread
 * */
static void * listen_thread(void * arg) {
    int listen_epfd = *((int*)arg);
    struct epoll_event new_client;
    /* Adding new Client Sock to the Workers' thread */
    int balance_index = 0;
    while (terminal_server != CLOSE_SERVE) {
        int is_work = epoll_wait(listen_epfd, &new_client, 1, 2000);
        int sock = 0;
        while (is_work > 0) { /* New Connect */
            sock = accept(new_client.data.fd, NULL, NULL);
            if (sock > 0) {
                set_nonblock(sock);
                clients[sock].file_dsp = sock;
                clients[sock].epfd_grop = epfd_group[balance_index];
                add_event(epfd_group[balance_index], sock, EPOLLIN);
                balance_index = (balance_index+1) % workers;
            } else /* sock == -1 means nothing to accept */
                break;
        } /* new Connect */
    }/* main while */
    close(listen_epfd);
    return NULL;
}
/* Workers' Thread
 * */
static void * workers_thread(void * arg) {
    int deal_epfd = *((int *)arg);
    struct epoll_event new_apply = {0};
    while(terminal_server != CLOSE_SERVE) {
        int is_apply = epoll_wait(deal_epfd, &new_apply, 1, 2000);
        if(is_apply > 0) { /* New Apply */
            int sock = new_apply.data.fd;
            conn_client * new_client = &clients[sock];
            if (new_apply.events & EPOLLIN) { /* Reading Work */
                /* TODO Handle read
                 * READ_STATUS  handle_read(conn_client *)
                 * PARSE_STATUS parse_read(conn_client *)
                 * */
                mod_event(deal_epfd, sock, EPOLLONESHOT | EPOLLOUT);
            }
            else if (new_apply.events & EPOLLOUT) { /* Writing Work */
                /* TODO Handle write
                 * WRITE_STATUS handle_write(conn_cliente *)
                 * */
                if(1 == new_client->linger)
                    mod_event(deal_epfd, sock, EPOLLONESHOT | EPOLLIN);
                else{
                    close(sock);
                    memset(new_client, 0, sizeof(conn_client));
                }
            }
            else { /* EPOLLRDHUG EPOLLERR EPOLLHUG */
                close(sock);
                memset(new_client, 0, sizeof(conn_client));
            }
        } /* New Apply */
    } /* main while */
    return NULL;
}

/* The Http server Main "Loop" */
void handle_loop(int file_dsption, int sock_type, const wsx_config_t * config) {
    workers = config->core_num - listeners;

    int listen_epfd = epoll_create1(0);
    if(-1 == listen_epfd) {
        perror("epoll_creat1: ");
        exit(-1);
    }
    add_event(listen_epfd, file_dsption, EPOLLIN | EPOLLHUP | EPOLLERR);

    /* Prepare Workers Sources */
    prepare_workers();

}

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
int open_listenfd(const char * restrict host_addr, const char * restrict port, int * restrict sock_type){
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

