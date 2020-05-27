
project(rmocks)
#
set(SOURCES
        log.mock.c
        log.mock.h
        mem.mock.h
        mem.mock.c
        os_wrapper.mock.h
        os_wrapper.mock.c
        list.mock.c list.mock.h thread.mock.h thread.mock.c system.mock.c system.mock.h)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
                      "${CMAKE_CURRENT_SOURCE_DIR}/../../include/r")

add_library(rmocks SHARED ${SOURCES})

target_include_directories(r
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}"
        )