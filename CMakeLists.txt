cmake_minimum_required(VERSION 2.6)
project(libr)

set(libr_VERSION_MAJOR 0)
set(libr_VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE DEBUG)

option(WITH_SGC "Compile the SGC library for memory leak detection" OFF)

set(SOURCES
  src/r.c 
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

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(r SHARED ${SOURCES})

target_include_directories(r
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/include/r"
        )

if(WITH_SGC)
  target_link_libraries(r pthread m sgc)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D MEM_DIAGNOSTICS")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -D MEM_DIAGNOSTICS ")
else(WITH_SGC)
  target_link_libraries(r pthread m)
endif(WITH_SGC)

INSTALL(TARGETS r DESTINATION "lib")
INSTALL (
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h")
