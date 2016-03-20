cmake_minimum_required(VERSION 3.3)
project(httpd3)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu99 -g -DWSX_DEBUG -pthread")
set(SOURCE_DIR config)
set(SOURCE_FILES config/wsx.conf main.c handle/handle.h handle/err_deal.h read_config.h read_config.c memop/manage.h memop/manage.c handle/handle.c handle/handle_read.h handle/handle_write.h handle/hancle_read.c handle/handle_core.h)

add_executable(httpd3 ${SOURCE_DIR} ${SOURCE_FILES} )