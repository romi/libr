#include <string>

#include "gtest/gtest.h"
#include "StringUtils.h"


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

TEST_F(string_utils_tests, string_ltrim_trims_left)
{
    // Arrange
    std::string three_spaces("   ");
    std::string expected = "string" + three_spaces;
    std::string input = three_spaces + expected;

    // Act
    std::string actual = StringUtils::ltrim(input, " ");

    //Assert
    ASSERT_EQ(actual, expected);
}

TEST_F(string_utils_tests, string_rtrim_trims_right)
{
    // Arrange
    std::string three_spaces("   ");
    std::string expected = three_spaces + "string";
    std::string input = expected + three_spaces;

    // Act
    std::string actual = StringUtils::rtrim(input, " ");

    //Assert
    ASSERT_EQ(actual, expected);
}

TEST_F(string_utils_tests, string_trim_trims_right_and_left)
{
    // Arrange
    std::string three_spaces("   ");
    std::string expected = "string";
    std::string input = three_spaces + expected + three_spaces;

    // Act
    std::string actual = StringUtils::trim(input, " ");

    //Assert
    ASSERT_EQ(actual, expected);
}

TEST_F(string_utils_tests, string_format_creates_expected_string)
{
    // Arrange
    const std::string format("%d %s");
    std::string expected("10 string");

    // Act
    std::string actual = StringUtils::string_format(format, 10, "string");

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
    StringUtils::string_printf(actual, format.c_str(), 10, "string");

    //Assert
    ASSERT_EQ(actual, expected);
}




