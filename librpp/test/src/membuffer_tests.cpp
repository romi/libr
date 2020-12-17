#include <string>

#include "gtest/gtest.h"
#include "MemBuffer.h"


class membuffer_tests : public ::testing::Test
{
protected:
    membuffer_tests() = default;

    ~membuffer_tests() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {

    }

};

TEST_F(membuffer_tests, string_format_creates_expected_string)
{
    // Arrange
    rpp::MemBuffer membuffer;
    std::string expected("test string is here");

    // Act
    membuffer.printf("%s", expected.c_str());
    auto buffer = membuffer.data();
    std::string actual(&buffer[0], &buffer[0] + buffer.size());

    //Assert
    ASSERT_EQ(actual, expected);
}
