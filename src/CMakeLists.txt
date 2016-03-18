cmake_minimum_required(VERSION 3.3)
project(httpd3)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu99 -g -DWSX_DEBUG")
set(SOURCE_DIR config)
set(SOURCE_FILES config/wsx.conf main.c handle/handle.h handle/err_deal.h read_config.h read_config.c memop/manage.h memop/manage.c handle/handle.c)

add_executable(httpd3 ${SOURCE_DIR} ${SOURCE_FILES} )