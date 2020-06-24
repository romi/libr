#include <string>
#include "gtest/gtest.h"

#include "json.h"

class json_tests : public ::testing::TestWithParam< std::tuple< std::string, std::string, std::string >>
{
protected:
    json_tests() = default;

    ~json_tests() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

std::tuple< std::string, std::string, std::string > json_test_parameters[] = {
        {"test y_number_0e+1.json", "y", "number"}
};

INSTANTIATE_TEST_CASE_P(run_json_parameterised_tests, json_tests, ::testing::ValuesIn(json_test_parameters));

TEST_P(json_tests, json_correctly_parses_test_file)
{
    // Arrange
    auto [ filename, result, type ] = GetParam();

    // Act

    //Assert
    ASSERT_EQ(filename, "test y_number_0e+1.json");
    ASSERT_EQ(result, "y");
    ASSERT_EQ(type, "number");
}
