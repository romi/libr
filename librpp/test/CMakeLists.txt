cmake_minimum_required(VERSION 3.10)

set(SRCS_RPP
        src/tests_main.cpp
        src/MemBuffer_tests.cpp
        src/StringUtils_tests.cpp
        src/json_cpp_tests.cpp
        src/FileUtils_tests.cpp
        mocks/Linux-Mock.cpp)

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

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME rpp_unit_tests_coverage
            EXECUTABLE ctest -V ${n_cores} -R "rpp_unit"
#            EXCLUDE '*wrapper*' "src/Linux.cpp" "include/Linux.h" "include/ILinux.h"
            EXCLUDE '*wrapper*' "src/Linux.cpp" "include/*.h"
            DEPENDENCIES rpp_unit_tests)
endif()
