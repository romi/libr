
project(rmocks)

set(SOURCES
        log.mock.c
        log.mock.h
        mem.mock.h
        mem.mock.c
        osw.mock.h
        osw.mock.c
        )

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
                      "${CMAKE_CURRENT_SOURCE_DIR}/../../include/r")

add_library(rmocks SHARED ${SOURCES})

target_include_directories(r
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}"
        )