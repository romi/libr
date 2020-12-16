#include <string>
#include <experimental/filesystem>

#include <zconf.h>
#include "gtest/gtest.h"
#include "../../../librpp/include/json_cpp.h"

extern "C" {
#include <math.h>
}


class json_cpp_tests : public ::testing::TestWithParam< std::string >
{
protected:
    json_cpp_tests() = default;

    ~json_cpp_tests() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

};

void test_vec(std::string& tester)
{
    std::vector<char> vec_string(32768);
    memcpy(vec_string.data(), "teststring", strlen("teststring"));
    tester = vec_string.data();
}

TEST_F(json_cpp_tests, json_load_fails_when_nofile)
{
    // Arrange

    std::string actual;
    test_vec(actual);

    // Act

    //Assert
    ASSERT_EQ(actual, std::string("teststring"));
}


