cmake_minimum_required(VERSION 2.6)
project(libr)

set(libr_VERSION_MAJOR 0)
set(libr_VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE DEBUG)

set(SOURCES
  src/mem.c 
  src/thread_pthread.c 
  src/fs_posix.c 
  src/clock_posix.c 
  src/membuf.c
  src/list.c 
  src/log.c 
  src/util.c
  src/json.c
  src/serial_posix.c
  )

include_directories(include)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(r SHARED ${SOURCES})
target_link_libraries(r pthread m)

INSTALL(TARGETS r DESTINATION "lib")
INSTALL (
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h")
