#include <string>
#include "FileUtils.h"

#include "gtest/gtest.h"
#include "StringUtils.h"


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

        remove(filename);

        // Act
        // Assert
        ASSERT_THROW(FileUtils::TryReadFileAsVector(filename, input), std::istream::failure);
}