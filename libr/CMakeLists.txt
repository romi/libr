cmake_minimum_required(VERSION 3.10)
project(libr)

############################################################

set_build_defaults()
output_compile_flags()

############################################################

set(libr_VERSION_MAJOR 0)
set(libr_VERSION_MINOR 1)

set(SOURCES
  src/mem.c
  src/r_mutex.c
  src/membuf.c
  src/log.c 
  src/util.c
  src/json.c
  src/os_wrapper.c
  include/r/os_wrapper.h)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(r SHARED ${SOURCES})

target_include_directories(r
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/include/r"
        )

target_link_libraries(r pthread)

# Always build the mocks library.
add_subdirectory(test/mocks)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
