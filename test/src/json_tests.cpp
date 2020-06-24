#include <string>
#include "gtest/gtest.h"
#include "test_file_utils.h"
#include "json.h"

class json_tests : public ::testing::TestWithParam< std::tuple< std::string, std::string, std::string >>
{
protected:
    json_tests() : json_directory("json_data/")
    {
    }

    ~json_tests() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
    const std::string json_directory;
};

std::tuple< std::string, std::string, std::string > json_test_parameters[] = {
        {"y_number_0e+1.json", "y", "number"}
};

INSTANTIATE_TEST_CASE_P(run_json_parameterised_tests, json_tests, ::testing::ValuesIn(json_test_parameters));

TEST_P(json_tests, json_correctly_parses_test_file)
{
    // Arrange
    auto [ filename, result, type ] = GetParam();
    std::string json_filename = json_directory + filename;
    std::string json_string;
    try {
        json_string = ReadFileAsString(json_filename);
    }
    catch(std::exception e)
    {
        std::cout << "Failed to load " << json_filename << std::endl;
    }

    // Act
    std::cout << json_string << std::endl;

    //Assert
    ASSERT_EQ(filename, "y_number_0e+1.json");
    ASSERT_EQ(result, "y");
    ASSERT_EQ(type, "number");
}
