# Initial test libr test check in. POC whilst exploring CMake downloading / install / build
# This is not integrated with the rest of the romi build, and will only build on the current, experimental, build system in-house.
cmake_minimum_required(VERSION 3.10)

set(SRCS
        src/tests_main.cpp
        src/list_tests.cpp)

add_executable( r_unit_tests
                ${SRCS})

target_link_libraries( r_unit_tests
                        gtest
                        gmock
                        r)

add_test(
    NAME r_unit_tests
    COMMAND r_unit_tests
)

if (BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_unit_tests_coverage
            BASE_DIRECTORY "../"
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests)
endif()