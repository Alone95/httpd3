//
// Created by root on 3/17/16.
//
#include "read_config.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>

int init_config(wsx_config_t * config){
    FILE * file = fopen(CONFIG_FILE_PATH, "r");
    char buf[PATH_LENGTH] = {"\0"};
    char * ret;
    if(NULL == file) {
        perror("Open File " CONFIG_FILE_PATH " : ");
        return -1; /* Can not read the Config file */
    }
    ret = fgets(buf, PATH_LENGTH, file);
    while (ret != NULL) {
#if defined(WSX_DEBUG_ALL)
        printf("\n\nReading \" %s \" into buf\n", buf);
#endif
        char * pos = strchr(buf, ':');
        char * check = strchr(buf, '#'); /* Start with # will be ignore */
        if (check != NULL)
            *check = '\0';

        if (pos != NULL) {
            *pos++ = '\0';
            if (0 == strncasecmp(buf, "thread", 6)) {
                sscanf(pos, "%d", &config->core_num);
#if defined(WSX_DEBUG)
                fprintf(stderr, "Thread number is %d \n", config->core_num);
#endif
            }
            else if (0 == strncasecmp(buf, "root", 4)) {
                sscanf(pos, "%s", &config->root_path);
#if defined(WSX_DEBUG)
                fprintf(stderr, "root path is %s\n", config->root_path);
#endif
            }
            else if (0 == strncasecmp(buf, "port", 4)) {
                sscanf(pos, "%s", &config->listen_port);
#if defined(WSX_DEBUG)
                fprintf(stderr, "listen port is %s\n", config->listen_port);
#endif
            }
            else if (0 == strncasecmp(buf, "addr", 4)) {
                sscanf(pos, "%s", &config->use_addr);
#if defined(WSX_DEBUG)
                fprintf(stderr, "Listen address is %s\n", config->use_addr);
#endif
            }
        }
        ret = fgets(buf, PATH_LENGTH, file);
    }
    fclose(file);
    return 0;
}