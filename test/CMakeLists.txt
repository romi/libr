# Initial test libr test check in. POC whilst exploring CMake downloading / install / build
# This is not integrated with the rest of the romi build, and will only build on the current, experimental, build system in-house.
cmake_minimum_required(VERSION 3.10)

set(SRCS_MEM
        src/tests_main.cpp
        src/mem_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c)


set(SRCS_COMMON
        src/tests_main.cpp
        src/list_tests.cpp
        src/clock_posix_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/mem.mock.h
        mocks/mem.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c)


add_executable( r_unit_tests_common
                ${SRCS_COMMON})

target_link_libraries( r_unit_tests_common
                        gtest
                        gmock
                        r)

add_executable( r_unit_tests_mem
                ${SRCS_MEM})

target_link_libraries( r_unit_tests_mem
                    gtest
                    gmock
                    r)

target_include_directories(r_unit_tests_common
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

target_include_directories(r_unit_tests_mem
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

add_test(
    NAME r_unit_tests_common
    COMMAND r_unit_tests_common
)

add_test(
        NAME r_unit_tests_mem
        COMMAND r_unit_tests_mem
)

SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME r_unit_tests_common_coverage
        EXECUTABLE ctest -V ${n_cores}
        DEPENDENCIES r_unit_tests_common)

SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME r_unit_tests_mem_coverage
        EXECUTABLE ctest -V ${n_cores}
        DEPENDENCIES r_unit_tests_mem)