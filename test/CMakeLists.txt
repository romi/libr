# Initial test libr test check in. POC whilst exploring CMake downloading / install / build
# This is not integrated with the rest of the romi build, and will only build on the current, experimental, build system in-house.
cmake_minimum_required(VERSION 3.10)

project(libr_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRCS_OS
        src/tests_main.cpp
        src/mem_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/memory_wrapper.mock.h
        mocks/memory_wrapper.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c)

set(SRCS_FS
        src/tests_main.cpp
        src/fs_posix_tests.cpp
        src/utils_tests.cpp
        support/test_file_utils.h
        support/test_file_utils.cpp
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
        mocks/mutex.mock.h
        mocks/mutex.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c)

set(SRCS_LOG_JSON
        src/tests_main.cpp
        src/log_tests.cpp
        src/json_tests.cpp
        support/test_file_utils.h
        support/test_file_utils.cpp)

set(SRCS_THREAD
        src/tests_main.cpp
        src/thread_pthread_tests.cpp
        mocks/pthread.condition.mock.h
        mocks/pthread.condition.mock.c
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/mem.mock.h
        mocks/mem.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c
        mocks/pthread.mutex.mock.h
        mocks/pthread.mutex.mock.c
        mocks/pthread.mock.h
        mocks/pthread.mock.c)

set(SRCS_SERIAL
        src/tests_main.cpp
        src/serial_posix_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/mutex.mock.h
        mocks/mutex.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c
        mocks/system.mock.h
        mocks/system.mock.c
        mocks/termios.mock.h
        mocks/termios.mock.c)

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

add_executable( r_unit_tests_log_json
        ${SRCS_LOG_JSON})

target_link_libraries( r_unit_tests_log_json
        gtest
        gmock
        r
        stdc++fs)

add_executable( r_unit_tests_thread
                ${SRCS_THREAD})

target_link_libraries( r_unit_tests_thread
        gtest
        gmock
        r)

add_executable( r_unit_tests_serial
        ${SRCS_SERIAL})

target_link_libraries( r_unit_tests_serial
        gtest
        gmock
        r
        stdc++fs)

target_include_directories(r_unit_tests_common
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_os
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_fs
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_log_json
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_thread
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_serial
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

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

add_test(
        NAME r_unit_tests_log_json
        COMMAND r_unit_tests_log_json
)

add_test(
        NAME r_unit_tests_thread
        COMMAND r_unit_tests_thread
)

add_test(
        NAME r_unit_tests_serial
        COMMAND r_unit_tests_serial
)

if(BUILD_COVERAGE)
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

    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_unit_tests_log_json_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests_log_json)

    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_unit_tests_thread_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests_thread)

    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_unit_tests_serial_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests_serial)
endif()

SET(JSON_INPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/json_data)

add_custom_target(build-time-make-directory ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${JSON_INPUT_DIR})

add_custom_command(TARGET r_unit_tests_log_json POST_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E copy
        json_data/*.json
        ${JSON_INPUT_DIR}
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS
            r_unit_tests_log_json
        COMMENT
        "Copying json test data to ${JSON_INPUT_DIR}"
        )