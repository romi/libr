cmake_minimum_required(VERSION 3.10)
project(librpp)

set(librpp_VERSION_MAJOR 0)
set(librpp_VERSION_MINOR 1)

#MESSAGE("librpp CXX Flags = ${CMAKE_CXX_FLAGS}")
#MESSAGE("librpp C Flags = ${CMAKE_C_FLAGS}")

set(SOURCES
        include/JsonCpp.h
        include/MemBuffer.h
        src/MemBuffer.cpp
        include/StringUtils.h
        src/StringUtils.cpp
        include/ILinux.h
        include/Linux.h
        src/Linux.cpp
        src/FileUtils.cpp
        include/FileUtils.h
        include/IClock.h
        include/Clock.h
        src/Clock.cpp
        include/ClockAccessor.h
        src/ClockAccessor.cpp)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(rpp SHARED ${SOURCES})

target_include_directories(rpp
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

target_link_libraries(rpp rppmocks r stdc++fs)

# Always build the mocks library.
add_subdirectory(test/mocks)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
