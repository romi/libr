# Initial test libr test check in. POC whilst exploring CMake downloading / install / build
# This is not integrated with the rest of the romi build, and will only build on the current, experimental, build system in-house.
cmake_minimum_required(VERSION 3.10)

set(SRCS_OS
        src/tests_main.cpp
        src/mem_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/memory_wrapper.mock.h
        mocks/memory_wrapper.mock.c
        mocks/os_wrapper.mock.h
        mocks/os_wrapper.mock.c)

set(SRCS_COMMON
        src/tests_main.cpp
        src/list_tests.cpp
        src/membuf_tests.cpp
        mocks/log.mock.h
        mocks/log.mock.c
        mocks/mem.mock.h
        mocks/mem.mock.c
        mocks/r_mutex.mock.h
        mocks/r_mutex.mock.c
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
        src/r_mutex_tests.cpp
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
        mocks/r_mutex.mock.h
        mocks/r_mutex.mock.c
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
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_os
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_log_json
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_thread
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
        ${CMAKE_CURRENT_SOURCE_DIR}/support)

target_include_directories(r_unit_tests_serial
        PRIVATE
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

add_test(NAME r_unit_tests_log_json
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
            NAME r_unit_tests_coverage
            EXECUTABLE ctest -V ${n_cores} -R "r_unit"
            EXCLUDE '*wrapper*'
            DEPENDENCIES r_unit_tests_log_json)
endif()

SET(JSON_INPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/json_data)

add_custom_command(TARGET r_unit_tests_log_json POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${JSON_INPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy json_data/*.json ${JSON_INPUT_DIR}
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS
            r_unit_tests_log_json
        COMMENT
        "Making directory ${JSON_INPUT_DIR} and copying JSON test files."
        )
