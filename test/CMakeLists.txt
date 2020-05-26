# Initial test libr test check in. POC whilst exploring CMake downloading / install / build
# This is not integrated with the rest of the romi build, and will only build on the current, experimental, build system in-house.
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRCS_OS
        src/tests_main.cpp
        src/mem_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c)

set(SRCS_FS
        src/tests_main.cpp
        src/fs_posix_tests.cpp
        src/utils_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c)

set(SRCS_COMMON
        src/tests_main.cpp
        src/list_tests.cpp
        src/clock_posix_tests.cpp
        src/membuf_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/mem.mock.h
        mocks/mem.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c
        mocks/thread.mock.h
        mocks/thread.mock.c)


add_executable( r_unit_tests_common
                ${SRCS_COMMON})

target_link_libraries( r_unit_tests_common
                        gtest
                        gmock
                        r)

add_executable( r_unit_tests_os
                ${SRCS_OS})

target_link_libraries( r_unit_tests_os
                    gtest
                    gmock
                    r)

add_executable( r_unit_tests_fs
        ${SRCS_FS})

target_link_libraries( r_unit_tests_fs
        gtest
        gmock
        r
        stdc++fs)

target_include_directories(r_unit_tests_common
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

target_include_directories(r_unit_tests_os
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

target_include_directories(r_unit_tests_fs
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

add_test(
    NAME r_unit_tests_common
    COMMAND r_unit_tests_common
)

add_test(
        NAME r_unit_tests_os
        COMMAND r_unit_tests_os
)

add_test(
        NAME r_unit_tests_fs
        COMMAND r_unit_tests_fs
)

SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME r_unit_tests_common_coverage
        EXECUTABLE ctest -V ${n_cores}
        DEPENDENCIES r_unit_tests_common)

SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME r_unit_tests_os_coverage
        EXECUTABLE ctest -V ${n_cores}
        DEPENDENCIES r_unit_tests_os)

SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME r_unit_tests_fs_coverage
        EXECUTABLE ctest -V ${n_cores}
        DEPENDENCIES r_unit_tests_fs)