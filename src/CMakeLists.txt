cmake_minimum_required(VERSION 3.3)
project(httpd3)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu99 -g -DWSX_DEBUG -pthread")
set(SOURCE_DIR config)
set(SOURCE_FILES config/wsx.conf
        read_config.h read_config.c main.c
        handle/handle.h memop/manage.h handle/handle_read.h handle/handle_write.h handle/http_response.h handle/handle_core.h
        handle/handle.c memop/manage.c handle/hancle_read.c                       handle/http_response.c
    )

add_executable(httpd3 ${SOURCE_DIR} ${SOURCE_FILES} )