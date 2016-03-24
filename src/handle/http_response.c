//
// Created by root on 3/21/16.
//
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include "http_response.h"
#include "../memop/manage.h"
extern website_root_path;
static const char * const
        ok_200_status[] = { "200",
                            "OK", ""};
static const char * const
        redir_304_status[] = {"304",
                              "Not Modify", "Your file is the Newest"};
static const char * const
        clierr_400_status[] = {"400",
                               "Bad Request", "WuShxin Server can not Resolve that Apply"};
static const char * const
        clierr_403_status[] = {"403",
                               "Forbidden", "Request Forbidden"};
static const char * const
        clierr_404_status[] = {"404",
                               "Not Found", "WuShxin Server can not Find the page"};
static const char * const
        clierr_405_status[] = {"405",
                               "Method Not Allow", "WuShxin Server has not implement that method yet ~_~"};
static const char * const
        sererr_500_status[] = {"500",
                               "Internal Server Error", "Apply Incomplete, WuShxin Server Meet Unknown Status"};
static const char * const
        sererr_503_status[] = {"503",
                               "Service Unavailable", "WuShxin Server is Overload Or go die : )"};
typedef enum {
    IS_NORMAL_FILE = 0,
    NO_SUCH_FILE = 1,
    FORBIDDEN_ENTRY = 2,
    IS_DIRECTORY = 3,
}URI_STATUS;
/*
 * Let the URI to be the Valid Way.
 * */
static inline void deal_uri(char * uri) {
#if defined(WSX_DEBUG)
    fprintf(stderr, "\nThe resource is %s\n", uri);
    fprintf(stderr, "Website_root_path: %s\n", website_root_path);
#endif
    if ('/' == uri[0] && uri[1] == '\0') { /*If Apply the root Resource  */
        snprintf(uri, strlen(website_root_path)+strlen("index.html")+1,"%sindex.html", website_root_path);
    }
    else {
        char tmp[1024] = {0};
        snprintf(tmp, 1024, "%s%s", website_root_path, uri+1);
        fprintf(stderr, "tmp : %s \n", tmp);
        strncpy(uri, tmp, strlen(tmp));
    }
#if defined(WSX_DEBUG)
    fprintf(stderr, "\nThe resource is %s\n", uri);
#endif
}
/*
 * Check if the Resource which CLient apply is A REAL File Resource(Include the Authorization)
 * */
static URI_STATUS check_uri(const char * uri, int * file_size) {
    struct stat buf = {0};
    if (stat(uri, &buf) < 0) {
        fprintf(stderr, "Get File %s ", uri);
        perror("Message: ");
        return NO_SUCH_FILE; /* No such file */
    }
    if ( 0 == (buf.st_mode & S_IROTH) ) {
        fprintf(stderr, "FORBIDDEN READING\n");
        return FORBIDDEN_ENTRY; /* Forbidden Reading */
    }
    if (S_ISDIR(buf.st_mode)) {
       return IS_DIRECTORY; /* Is Directiry */
    }
    *file_size = buf.st_size;
    return IS_NORMAL_FILE;
}
/*
 * Put the Respone Message to the conn_client's write_buf
 * return 0 if Success, Or the remaining Bytes that can not pull into the write buffer(Not Big Enough)
 *
 * */
static int write_to_buf(conn_client * client,
                         const char ** status, const char * http_ver,
                         const char * source_path, int source_size) {
    char * write_buf = client->write_buf;
    int w_count = 0;
    w_count += snprintf(write_buf+w_count, BUF_SIZE-w_count, "%s %s %s\r\n", "HTTP/1.0", status[0], status[1]);
    w_count += snprintf(write_buf+w_count, BUF_SIZE-w_count, "Content-Length: %d\r\n", source_path == NULL ? strlen(status[2]):source_size);
    w_count += snprintf(write_buf+w_count, BUF_SIZE-w_count, "Connection: close\r\n");
    w_count += snprintf(write_buf+w_count, BUF_SIZE-w_count, "\r\n");
    if (NULL == source_path) {
        snprintf(write_buf+w_count, BUF_SIZE-w_count, status[2]);
        return 0;
    }
    int fd = open(source_path, O_RDONLY);
    if (fd < 0) {
        return -1; /* Write again */
    }
    char * file_map = mmap(NULL, source_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (NULL == file_map) {
        assert(file_map != NULL);
    }
    close(fd);
    int content_w = snprintf(write_buf+w_count, BUF_SIZE-w_count, file_map);
    client->write_offset = content_w + w_count;
    munmap(file_map, source_size);
    if (content_w < source_size) {
        fprintf(stderr, "File has not send completely, Still has %d bytes\n", source_size - content_w);
        return source_size - content_w;
        /* TODO
         * Add a Extra Buffer to Store the Part of Data(Using heap allocation)
         * Set the bit(Check bit) in the pointer which points to the Buffer
         * */
    }
    return 0;
}
/*
 * Universal Response Page maker
 * */
MAKE_PAGE_STATUS make_response_page(const conn_client * client,
                                    const char * http_ver, const char * uri, const char * method)
{
    int err_code = 0;
    int uri_file_size = 0;
    int len_source = strlen(uri);
    char * source = Malloc(len_source > 1024 ? len_source : 1024);
    strncpy(source, uri, len_source);
    if (uri != NULL && source != NULL)
        deal_uri(source);
    else{
        Free(source);
SERVER_ERROR:
        write_to_buf(client, sererr_500_status, http_ver, NULL, 0);
        return MAKE_PAGE_FAIL;
    }
    if(0 == strncasecmp(method, "GET", 3)) {
        err_code = check_uri(source, &uri_file_size);
        if (IS_NORMAL_FILE == err_code) {
            if((err_code = write_to_buf(client, ok_200_status, http_ver, source, uri_file_size)) < 0)
                goto SERVER_ERROR;
        }else if (FORBIDDEN_ENTRY == err_code){
            write_to_buf(client, clierr_403_status, http_ver, NULL, 0);
        }
        else /*if (NO_SUCH_FILE == err_code || IS_DIRECTORY == err_code)*/ {
            write_to_buf(client, clierr_404_status, http_ver, NULL, 0);
        }

    }
    else if ( 0 == strncasecmp(method, "POST", 4)) {
        /* TODO POST Method */
        write_to_buf(client, clierr_405_status, http_ver, NULL, 0);
    }
    else { /* Unknown Method */
        write_to_buf(client, clierr_400_status, http_ver, NULL, 0);
    }
    Free(source);
    return MAKE_PAGE_SUCCESS;
}