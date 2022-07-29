cmake_minimum_required(VERSION 3.10)
project(librpp)

############################################################

output_compile_flags()

############################################################

set(librpp_VERSION_MAJOR 0)
set(librpp_VERSION_MINOR 1)

set(SOURCES
        include/IThreadsafeQueue.h
        include/ThreadsafeQueue.h
#        include/JsonCpp.h
        include/StringUtils.h
        src/StringUtils.cpp
        include/ILinux.h
        include/Linux.h
        src/Linux.cpp
        include/ILogWriter.h
        include/LogWriter.h
        include/ILogger.h
        include/Logger.h
        src/Logger.cpp
        src/LogWriter.cpp
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

target_link_libraries(rpp rppmocks stdc++fs)

# Always build the mocks library.
add_subdirectory(test/mocks)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
