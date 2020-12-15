#include <string>

#include "gtest/gtest.h"
#include "string_utils.h"


class string_utils_tests : public ::testing::Test
{
protected:
    string_utils_tests() = default;

    ~string_utils_tests() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {

    }

};

TEST_F(string_utils_tests, string_format_creates_expected_string)
{
    // Arrange
    const std::string format("%d %s");
    std::string expected("10 string");

    // Act
    std::string actual = rpp::string_format(format, 10, "string");

    //Assert
    ASSERT_EQ(actual, expected);
}

TEST_F(string_utils_tests, string_printf_creates_expected_string)
{
    // Arrange
    const std::string format("%d %s");
    std::string expected("10 string");
    std::string actual;

    // Act
    rpp::string_printf(actual, format.c_str(), 10, "string");

    //Assert
    ASSERT_EQ(actual, expected);
}



