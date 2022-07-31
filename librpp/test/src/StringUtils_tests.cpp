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


TEST_F(string_utils_tests, rprintf_with_long_enough_buffer_prints_to_buffer)
{
    // Arrange
    const int buffsize = 128;
    char buffer[buffsize];
    memset(buffer, 0, buffsize);
    std::string expected("string 10 string 0x01");

    // Act
    char * res = StringUtils::rprintf(buffer, buffsize, "%s %d %s %#04x", "string", 10, "string", 1);
    std::string actual(buffer);

    // Assert
    ASSERT_EQ(res, buffer);
    ASSERT_EQ(actual, expected);
}

TEST_F(string_utils_tests, rprintf_with_small_buffer_returns_null)
{
    // Arrange
    const int buffsize = 6;
    char buffer[buffsize];
    memset(buffer, 0, buffsize);
    std::string expected("string 10 string 0x01");

    // Act
    char * res = StringUtils::rprintf(buffer, buffsize, "%s %d %s %#04x", "string", 10, "string", 1);
    std::string actual(buffer);

    // Assert
    ASSERT_EQ(res, nullptr);
}



