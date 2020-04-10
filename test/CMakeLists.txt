# Initial test libr test check in. POC whilst exploring CMake downloading / install / build
# This is not integrated with the rest of the romi build, and will only build on the current, experimental, build system in-house.
cmake_minimum_required(VERSION 3.10)

add_executable( r_unit_tests
                src/tests_main.cpp
                src/list_tests.cpp
                )

target_link_libraries( r_unit_tests
                        gtest
                        gmock
                        r)

target_include_directories(r_unit_tests  PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}/../include/r
                            ${CMAKE_CURRENT_SOURCE_DIR}/../include
                            ${CMAKE_CURRENT_SOURCE_DIR})

add_test(
    NAME r_unit_tests
    COMMAND r_unit_tests
)

SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME r_unit_tests_coverage
        EXECUTABLE ctest -V ${n_cores} # Executable in PROJECT_BINARY_DIR
        DEPENDENCIES
        r_unit_tests)
