cmake_minimum_required(VERSION 3.10)

project(libr)
set(libr_VERSION_MAJOR 0)
set(libr_VERSION_MINOR 1)

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
  src/os_wrapper.c
  include/r/os_wrapper.h)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(r SHARED ${SOURCES})

target_include_directories(r
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/include/r"
        )

#target_link_libraries(r pthread m)
target_link_libraries(r pthread)

# Always build the mocks library.
add_subdirectory(test/mocks)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
