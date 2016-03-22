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
static inline void deal_uri(char * uri) {
    fprintf(stderr, "\nThe resource is %s\n", uri);
    if ('/' == uri[0] && uri[1] == '\0') {
        snprintf(uri, strlen(uri)+1,"%sindex.html", website_root_path);
    }
    else {
        char tmp[1024] = {0};
        snprintf(tmp, 1024, "%s%s", website_root_path, uri+1);
        fprintf(stderr, "tmp : %s \n", tmp);
        strncpy(uri, tmp, strlen(tmp));
    }
    fprintf(stderr, "\nThe resource is %s\n", uri);
}
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
static int write_to_buf(const conn_client * client,
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
        assert(fd > 0);
    }
    char * file_map = mmap(NULL, source_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (NULL == file_map) {
        assert(file_map != NULL);
    }
    close(fd);
    int content_w = snprintf(write_buf+w_count, BUF_SIZE-w_count, file_map);
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
    else
        assert(0);
    if(0 == strncasecmp(method, "GET", 3)) {
        err_code = check_uri(source, &uri_file_size);
        if (err_code != IS_NORMAL_FILE) {
            write_to_buf(client, clierr_404_status, http_ver, NULL, uri_file_size);
        }
        else
            write_to_buf(client, ok_200_status, http_ver, source, uri_file_size);
    }
    else if ( 0 == strncasecmp(method, "POST", 4)) {
        /* TODO */
    }
    else {

    }
    Free(source);
    return MAKE_PAGE_SUCCESS;
}