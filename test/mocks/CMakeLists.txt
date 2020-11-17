
project(rmocks)
#
set(SOURCES
        mem.mock.h
        mem.mock.c)

#set(SOURCES
#        fcntl.mock.h
#        fcntl.mock.c
#        list.mock.h
#        list.mock.c
#        log.mock.h
#        log.mock.c
#        mem.mock.h
#        mem.mock.c
#        membuf.mock.h
#        membuf.mock.c
#        mutex.mock.h
#        mutex.mock.c
#        os_wrapper.mock.h
#        os_wrapper.mock.c
#        list.mock.c
#        list.mock.h
#        mutex.mock.h
#        mutex.mock.c
#        system.mock.c
#        system.mock.h)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
                      "${CMAKE_CURRENT_SOURCE_DIR}/../../include/r")

add_library(rmocks SHARED ${SOURCES})

target_include_directories(rmocks
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}"
        )