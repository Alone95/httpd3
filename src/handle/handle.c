//
// Created by root on 3/17/16.
//

#include "handle.h"
#include <linux/tcp.h>
#include <pthread.h>

#include <assert.h>
#include <signal.h>
#if defined(WSX_DEBUG)
#endif
enum SERVE_STATUS {
    CLOSE_SERVE = 1,
    RUNNING_SERVER= 0
};
static enum SERVE_STATUS terminal_server = RUNNING_SERVER;
char * website_root_path = NULL;
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
static void prepare_workers(wsx_config_t * config) {
    epfd_group_size = workers;
    epfd_group = Malloc(epfd_group_size * (sizeof(int)));
#if defined(WSX_DEBUG)
    if (NULL == epfd_group) {
        fputs( "Malloc Fail!", stderr);
        assert(NULL != epfd_group);
    }
#endif
    for (int i = 0; i < epfd_group_size; ++i) {
        epfd_group[i] = epoll_create(5);
#if defined(WSX_DEBUG)
        if (-1 == epfd_group[i])
            fprintf(stderr, "Error on epoll\n");
        //assert(-1 == epfd_group[i]);
#endif
    }
    clients = Malloc(OPEN_FILE * sizeof(conn_client));
#if defined(WSX_DEBUG)
    if (NULL == clients) {
        fputs( "Malloc Fail!", stderr);
        assert(NULL != clients);
    }
#endif
    website_root_path = Malloc(strlen(config->root_path)+1);
    if (NULL == website_root_path) {
        fputs( "Malloc Fail!", stderr);
        assert(NULL != website_root_path);
    }
    else {
        strncpy(website_root_path, config->root_path, strlen(config->root_path));
    }
}
static inline void destroy_resouce() {
    Free(epfd_group);
    Free(clients);
    Free(website_root_path);
}
/* Listener's Thread
 * */
static void * listen_thread(void * arg) {
    int listen_epfd = (int)arg;
    struct epoll_event new_client;
    /* Adding new Client Sock to the Workers' thread */
    int balance_index = 0;
    while (terminal_server != CLOSE_SERVE) {
        int is_work = epoll_wait(listen_epfd, &new_client, 1, 2000);
        int sock = 0;
        while (is_work > 0) { /* New Connect */
            sock = accept(new_client.data.fd, NULL, NULL);
            if (sock > 0) {
                fprintf(stderr, "There has a client(%d) Connected\n", sock);
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

    pthread_exit(0);
}
/* Workers' Thread
 * */
static void * workers_thread(void * arg) {
    int deal_epfd = (int)arg;
    struct epoll_event new_apply = {0};
    while(terminal_server != CLOSE_SERVE) {
        int is_apply = epoll_wait(deal_epfd, &new_apply, 1, 2000);
        if(is_apply > 0) { /* New Apply */
            int sock = new_apply.data.fd;
            conn_client * new_client = &clients[sock];
            fprintf(stderr, "The thread %d receive the client(%d)\n", pthread_self(), sock);
            if (new_apply.events & EPOLLIN) { /* Reading Work */
                int err_code = handle_read(new_client);
                if (err_code != HANDLE_READ_SUCCESS)
                    fprintf(stderr, "READ FROM NEW CLIENT FAIL\n");
                mod_event(deal_epfd, sock, EPOLLONESHOT | EPOLLOUT);
                fprintf(stderr, "Client(%d)Read For Writing!!: \n\n%s",new_client->file_dsp, new_client->write_buf);
                /* TODO Handle read
                 * READ_STATUS  handle_read(conn_client *)
                 * PARSE_STATUS parse_read(conn_client *)
                 * */
            }
            else if (new_apply.events & EPOLLOUT) { /* Writing Work */
                int err_code = handle_write(new_client);
                if (HANDLE_WRITE_AGAIN == err_code) /* TCP's Write buffer is Busy */
                    mod_event(deal_epfd, sock, EPOLLONESHOT | EPOLLOUT);
                else if (HANDLE_READ_FAILURE == err_code){ /* Peer Close */
                    close(sock);
                    memset(new_client, 0, sizeof(conn_client));
                    continue;
                }
                /* if Keep-alive */
                if(1 == new_client->linger)
                    mod_event(deal_epfd, sock, EPOLLONESHOT | EPOLLIN);
                else{
                    close(sock);
                    memset(new_client, 0, sizeof(conn_client));
                    continue;
                }
            }
            else { /* EPOLLRDHUG EPOLLERR EPOLLHUG */
                close(sock);
                memset(new_client, 0, sizeof(conn_client));
            }
        } /* New Apply */
    } /* main while */
    pthread_exit(1);
}
static void shutdowns(int arg) {
    terminal_server = CLOSE_SERVE;
    return;
}
/* The Http server Main "Loop" */
void handle_loop(int file_dsption, int sock_type, const wsx_config_t * config) {
    signal(SIGINT, shutdowns);
    workers = config->core_num - listeners;

    int listen_epfd = epoll_create1(0);
    if(-1 == listen_epfd) {
        perror("epoll_creat1: ");
        exit(-1);
    }
    { /* Register listen fd to the listen_epfd */
        struct epoll_event event;
        event.data.fd = file_dsption;
        event.events = EPOLLET | EPOLLERR | EPOLLIN;
        epoll_ctl(listen_epfd, EPOLL_CTL_ADD, file_dsption, &event);
    }
    /* Prepare Workers Sources */
    prepare_workers(config);
    pthread_t listener_set[listeners];
    pthread_t worker_set[workers];
    for (int i = 0; i < listeners; ++i) {
        pthread_create(&listener_set[i], NULL, listen_thread, (void*)listen_epfd);
        //pthread_detach(listener_set[i]);
    }
    for (int j = 0; j < workers; ++j) {
        pthread_create(&worker_set[j], NULL, workers_thread, (void*)(epfd_group[j]));
        pthread_detach(worker_set[j]);
    }
    for (int k = 0; k < listeners; ++k) {
        pthread_join(listener_set[k], NULL);
    }
    destroy_resouce();
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

