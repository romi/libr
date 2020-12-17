cmake_minimum_required(VERSION 3.10)

project(librpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(librpp_VERSION_MAJOR 0)
set(librpp_VERSION_MINOR 1)
#
## Default option is off.
## We override this on the server for a test buid, so tests should be run with this on before checking in.
#option(ADDRESS_SANITISER_BUILD "Enable build settings to allow address sanitiser" ON) # default to off
#
#if(ADDRESS_SANITISER_BUILD)
#    message(WARNING "Enabling GCC address sanitiser will disable all Valgrind tasks due to compatibility errors")
#    set( PROJECT_SANITISE_FLAGS
#            "-fsanitize=address,undefined,shift,integer-divide-by-zero,unreachable,vla-bound,null,return,signed-integer-overflow,bounds,alignment,object-size,float-divide-by-zero,float-cast-overflow,nonnull-attribute,returns-nonnull-attribute,bool,enum,vptr -fno-omit-frame-pointer")
#else()
#    MESSAGE("libr sanitize flags off")
#    set( PROJECT_SANITISE_FLAGS "")
#endif()
#
## For now everything is debug. Set Compile options locally to maintain independent library builds.
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror ${PROJECT_SANITISE_FLAGS}")
#set(CMAKE_CXX_FLAGS_DEBUG "-O0")
#set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror ${PROJECT_SANITISE_FLAGS}")
#set(CMAKE_C_FLAGS_DEBUG "-O0")
#
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROJECT_SANITISE_FLAGS}")
#
#############################################################
#
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

############################################################


# Options.
# Build tests by default. We want to run tests whether we are developing or building a release. I mean... why wouldn't you? :)
option(BUILD_TESTS "Build all tests." ON)
option(BUILD_COVERAGE "Build coverage." ON)

set(SOURCES
        include/json_cpp.h
        include/membuffer.h
        src/membuffer.cpp
        include/string_utils.h
        src/string_utils.cpp
        include/ILinux.h
        include/Linux.h
        src/Linux.cpp)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(rpp SHARED ${SOURCES})

target_include_directories(rpp
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

#target_link_libraries(r pthread m)
target_link_libraries(rpp r)

# Always build the mocks library.
#add_subdirectory(test/mocks)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
