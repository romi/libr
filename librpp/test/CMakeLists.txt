cmake_minimum_required(VERSION 3.10)

set(SRCS_RPP
        src/tests_main.cpp
        src/StringUtils_tests.cpp
#        src/json_cpp_tests.cpp
        src/FileUtils_tests.cpp
        src/Logger_tests.cpp
        mocks/mock_linux.h)

add_executable( rpp_unit_tests
                ${SRCS_RPP})

target_link_libraries( rpp_unit_tests
                        gtest
                        gmock
                        rpp)

add_test(
    NAME rpp_unit_tests
    COMMAND rpp_unit_tests
)

set(SRCS_RPP_CLOCK
        src/tests_main.cpp
        src/Clock_tests.cpp
        )

add_executable( rpp_clock_unit_tests
        ${SRCS_RPP_CLOCK})

target_link_libraries( rpp_clock_unit_tests
        gtest
        gmock
        rpp
        rppmocks)

add_test(
        NAME rpp_clock_unit_tests
        COMMAND rpp_clock_unit_tests
)

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME rpp_unit_tests_coverage
            EXECUTABLE ctest -V ${n_cores} -R "rpp_"
            EXCLUDE '*wrapper*' "src/Linux.cpp" "include/*.h"
            DEPENDENCIES rpp_unit_tests)
endif()
