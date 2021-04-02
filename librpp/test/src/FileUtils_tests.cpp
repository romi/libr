#include <string>
#include "FileUtils.h"

#include "gtest/gtest.h"
#include "StringUtils.h"
#include "mock_linux.h"

using namespace testing;

class file_utils_tests : public ::testing::Test
{
protected:
    file_utils_tests() = default;

    ~file_utils_tests() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {

    }

};

TEST_F(file_utils_tests, write_vector_as_uint8_succeeds)
{
        // Arrange
        std::vector<uint8_t> output = {0,1,2,3,4,5};
        std::vector<uint8_t> input{};
        const char *filename = "uint8.vec";

        remove(filename);

        // Act
        ASSERT_NO_THROW(FileUtils::TryWriteVectorAsFile(filename, output));
        ASSERT_NO_THROW(FileUtils::TryReadFileAsVector(filename, input));

        //Assert
        ASSERT_EQ(output,input);
}


TEST_F(file_utils_tests, write_vector_as_uint8_throws_on_fail)
{
        // Arrange
        std::vector<uint8_t> output = {0,1,2,3,4,5};
        std::vector<uint8_t> input{};
        const char *filename = "/root/fail/uint8.vec";

        remove(filename);

        // Act
        // Assert
        ASSERT_THROW(FileUtils::TryWriteVectorAsFile(filename, output), std::ostream::failure);
}

TEST_F(file_utils_tests, read_vector_as_uint8_throws_on_fail)
{
        // Arrange
        std::vector<uint8_t> output = {0,1,2,3,4,5};
        std::vector<uint8_t> input{};
        const char *filename = "/root/fail/uint8.vec";

        // Act
        // Assert
        ASSERT_THROW(FileUtils::TryReadFileAsVector(filename, input), std::istream::failure);
}

TEST_F(file_utils_tests, write_string_succeeds)
{
        // Arrange
        std::string output("This is a string");
        std::string input{};
        const char *filename = "string.txt";

        remove(filename);

        // Act
        ASSERT_NO_THROW(FileUtils::TryWriteStringAsFile(filename, output));
        ASSERT_NO_THROW(input = FileUtils::TryReadFileAsString(filename));

        //Assert
        ASSERT_EQ(output,input);
}

TEST_F(file_utils_tests, write_string_throws_on_fail)
{
        // Arrange
        std::string output("This is a string");
        const char *filename = "/root/fail/string.txt";

        // Act
        ASSERT_THROW(FileUtils::TryWriteStringAsFile(filename, output), std::ostream::failure);
}

TEST_F(file_utils_tests, read_string_throws_on_fail)
{
        // Arrange
        std::string output("This is a string");
        const char *filename = "string.txt";
        remove(filename);

        // Act
        ASSERT_THROW(FileUtils::TryReadFileAsString(filename),std::ostream::failure );
}


TEST_F(file_utils_tests, get_home_dir_get_env_succeeds_returns_homedir)
{
        // Arrange
        rpp::MockLinux mock_linux;
        std::string expected("/home");
        EXPECT_CALL(mock_linux, secure_getenv)
                        .WillOnce(Return((char*)expected.c_str()));
        // Act
        auto actual = FileUtils::TryGetHomeDirectory(mock_linux);

        // Assert
        ASSERT_EQ(actual, expected);
}

TEST_F(file_utils_tests, get_home_dir_get_env_fails_getpuuid_succeeds_returns_homedir)
{
        // Arrange
        rpp::MockLinux mock_linux;
        std::string expected("/home");
        struct passwd password{};
        password.pw_dir = (char*)expected.c_str();

        EXPECT_CALL(mock_linux, secure_getenv)
                        .WillOnce(Return(nullptr));
        EXPECT_CALL(mock_linux, getuid)
                        .WillOnce(Return(0));
        EXPECT_CALL(mock_linux, getpwuid)
                        .WillOnce(Return(&password));
        // Act
        auto actual = FileUtils::TryGetHomeDirectory(mock_linux);

        // Assert
        ASSERT_EQ(actual, expected);
}

TEST_F(file_utils_tests, get_home_dir_get_env_fails_getpuuid_fails_throws_exception)
{
        // Arrange
        rpp::MockLinux mock_linux;
        struct passwd password{};
        password.pw_dir = nullptr;
        bool exception_thrown = false;

        EXPECT_CALL(mock_linux, secure_getenv)
                        .WillOnce(Return(nullptr));
        EXPECT_CALL(mock_linux, getuid)
                        .WillOnce(Return(0));
        EXPECT_CALL(mock_linux, getpwuid)
                        .WillOnce(Return(&password));
        // Act
        try {
                FileUtils::TryGetHomeDirectory(mock_linux);
        }
        catch (std::exception& e)
        {
                exception_thrown = true;
        }

        // Assert
        ASSERT_TRUE(exception_thrown);
}