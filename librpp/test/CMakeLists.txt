cmake_minimum_required(VERSION 3.10)

set(SRCS_RPP
        src/tests_main.cpp
        src/membuffer_tests.cpp
        src/string_utils_tests.cpp)

add_executable( rpp_unit_tests_common
                ${SRCS_RPP})

target_link_libraries( rpp_unit_tests_common
                        gtest
                        gmock
                        rpp
                        r)

add_test(
    NAME rpp_unit_tests_common
    COMMAND rpp_unit_tests_common
)

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME rpp_unit_tests_common_coverage
            EXECUTABLE ctest -V ${n_cores} -R "rpp_unit"
            EXCLUDE '*wrapper*'
            DEPENDENCIES rpp_unit_tests_common)
endif()
