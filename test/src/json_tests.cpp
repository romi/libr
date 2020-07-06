#include <string>
#include <experimental/filesystem>

#include <zconf.h>
#include "gtest/gtest.h"
#include "test_file_utils.h"
#include "json.h"

extern "C" {
#include <math.h>
}

extern std::string full_exe_path;

namespace fs = std::experimental::filesystem;


static std::string getexepath()
{
    char result[ PATH_MAX ];
    ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    std::string sresult(result, (count > 0) ? count : 0 );
    std::string pstring = fs::path(result).parent_path();
    return pstring;
}

const std::string json_directory = getexepath() + "/json_data/";

class json_tests : public ::testing::TestWithParam< std::string >
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

public:
};

std::vector<std::string> GetFilesInDir(std::string directory)
{
    std::vector<std::string> filenames;

    for(auto& p : fs::directory_iterator(directory))
    {
        filenames.emplace_back(p.path().filename());
    }
    return filenames;
}

std::pair<std::string, std::string>
parse_json_filename(std::string json_test_filename)
{
    std::string delim = "_";

    auto start = 0U;
    auto end = json_test_filename.find(delim);
    std::string success = json_test_filename.substr(start, end - start);
    start = end + delim.length();
    end = json_test_filename.find(delim, start);
    std::string type = json_test_filename.substr(start, end - start);
    return std::make_pair(success, type);

}

int get_expected_error(std::string success)
{
    int successval = 1;
    if (success == "y")
        successval = 0;
    return successval;
}

int isnumber(json_object_t obj)
{
    return json_isnumber(obj)
           || (json_isarray(obj)
               && json_array_length(obj) == 1
               && json_isnumber(json_array_get(obj, 0)));
}

double get_number(json_object_t obj)
{
    double ret = json_array_getnum(obj, 0);
    return ret;
}

int isstring(json_object_t obj)
{
    return json_isstring(obj)
           || (json_isarray(obj)
               && json_array_length(obj) == 1
               && json_isstring(json_array_get(obj, 0)));
}

const char *get_string(json_object_t obj)
{
    return json_array_getstr(obj, 0);
}


bool is_type(std::string type, json_object_t obj)
{
    bool ret = false;

    if (type.find("number") != std::string::npos)
        ret = isnumber(obj);
    else if (type.find("string") != std::string::npos)
        ret = isstring(obj);
    else if (type.find("object") != std::string::npos)
        ret = json_isobject(obj);
    else if (type.find("array") != std::string::npos)
        ret = json_isarray(obj);
    else if (type.find("structure") != std::string::npos)
        ret = true;

    return ret;
}

INSTANTIATE_TEST_SUITE_P(run_json_parameterised_tests, json_tests, ::testing::ValuesIn(GetFilesInDir(json_directory)));

TEST_P(json_tests, json_correctly_parses_test_file)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string filename = GetParam();
    std::string json_filename = json_directory + filename;
    auto success_and_type = parse_json_filename(filename);
    auto expected_err = get_expected_error(success_and_type.first);

    // Act
    auto obj = json_load(json_filename.c_str(), &parse_error, error_message, sizeof(error_message));
    bool expected_type = is_type(success_and_type.second, obj);

    //Assert
    if (success_and_type.first == "y") {
        ASSERT_EQ(expected_err, 0);
        ASSERT_TRUE(expected_type);
    }
    else
        ASSERT_NE(expected_err, 0);

    json_unref(obj);
}

TEST_P(json_tests, json_correctly_parses_test_string)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string filename = GetParam();
    std::string json_filename = json_directory + filename;
    auto success_and_type = parse_json_filename(filename);
    auto expected_err = get_expected_error(success_and_type.first);

    // Act
    std::string json_string = ReadFileAsString(json_filename);
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));

    bool expected_type = is_type(success_and_type.second, obj);

    //Assert
    if (success_and_type.first == "y") {
        ASSERT_EQ(expected_err, 0);
        ASSERT_TRUE(expected_type);
    }
    else
        ASSERT_NE(expected_err, 0);

    json_unref(obj);
}

//TEST_F(json_tests, json_correctly_parses_test_file_specific)
//{
//    // Arrange
//    int parse_error = 0;
//    char error_message[256];
//
//    std::string filename("y_number_negative_one.json");
//    std::cout << filename << std::endl;
//    std::string json_filename = json_directory + filename;
//    auto success_and_type = parse_json_filename(filename);
//    auto expected_err = get_expected_error(success_and_type.first);
//
//    // Act
//    auto obj = json_load(json_filename.c_str(), &parse_error, error_message, sizeof(error_message));
//    bool expected_type = is_type(success_and_type.second, obj);
//    std::cout << "Object_type " << obj->type << std::endl;
//
//    //Assert
//    if (success_and_type.first == "y") {
//        ASSERT_EQ(expected_err, 0);
//        ASSERT_TRUE(expected_type);
//    }
//    else
//        ASSERT_NE(expected_err, 0);
//
//    json_unref(obj);
//}


TEST_F(json_tests, json_number_value_correct_when_valid)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    double expected_number = -1;

    std::string filename("y_number_negative_one.json");
    std::string json_filename = json_directory + filename;
    auto success_and_type = parse_json_filename(filename);

    // Act
    auto obj = json_load(json_filename.c_str(), &parse_error, error_message, sizeof(error_message));
    double actual_number = get_number(obj);
    json_unref(obj);

    //Assert
    ASSERT_EQ(expected_number, actual_number);
}

TEST_F(json_tests, json_number_value_correct_when_invalid)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string filename("y_string_simple_ascii.json");
    std::string json_filename = json_directory + filename;
    auto success_and_type = parse_json_filename(filename);

    // Act
    auto obj = json_load(json_filename.c_str(), &parse_error, error_message, sizeof(error_message));
    double actual_number = get_number(obj);
    json_unref(obj);

    //Assert
    ASSERT_TRUE(isnan(actual_number));
}

TEST_F(json_tests, json_string_value_correct_when_valid)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string filename("y_string_simple_ascii.json");
    std::string json_filename = json_directory + filename;
    auto success_and_type = parse_json_filename(filename);
    std::string expected_string = "asd ";

    // Act
    auto obj = json_load(json_filename.c_str(), &parse_error, error_message, sizeof(error_message));
    std::string actual_string = get_string(obj);
    json_unref(obj);

    //Assert
    ASSERT_EQ(expected_string, actual_string);
}

TEST_F(json_tests, json_string_value_correct_when_not_string_in_array)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string filename("y_number_negative_one.json");
    std::string json_filename = json_directory + filename;
    auto success_and_type = parse_json_filename(filename);

    // Act
    auto obj = json_load(json_filename.c_str(), &parse_error, error_message, sizeof(error_message));
    const char *actual_string = get_string(obj);
    json_unref(obj);

    //Assert
    ASSERT_EQ(actual_string, nullptr);
}

TEST_F(json_tests, json_string_value_correct_when_not_string)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string filename("y_number_negative_one.json");
    std::string json_filename = json_directory + filename;
    auto success_and_type = parse_json_filename(filename);

    // Act
    auto obj = json_load(json_filename.c_str(), &parse_error, error_message, sizeof(error_message));
    const char *actual_string = json_string_value(obj);
    json_unref(obj);

    //Assert
    ASSERT_EQ(actual_string, nullptr);
}

TEST_F(json_tests, json_string_length_correct)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("[\"test_string\"]");
    int expected_json_string_length = json_string.length()- 4; // []""

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    int actual_length = json_string_length(json_array_get(obj, 0));
    json_unref(obj);

    //Assert
    ASSERT_EQ(actual_length, expected_json_string_length);
}

TEST_F(json_tests, json_string_length_correct_when_not_a_string)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("[\"test_string\"]");
    int expected_json_string_length = 0;

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    int actual_length = json_string_length(obj);
    json_unref(obj);

    //Assert
    ASSERT_EQ(actual_length, expected_json_string_length);
}

TEST_F(json_tests, json_string_compare_with_equal_strings_returns_non_0)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("[\"test_string\"]");
    std::string test_string("test_string");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    int actual = json_string_equals(json_array_get(obj, 0), test_string.c_str());
    json_unref(obj);

    //Assert
    ASSERT_NE(actual, 0);
}

TEST_F(json_tests, json_string_compare_with_non_equal_strings_returns_0)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("[\"test_string\"]");
    std::string json_compare_string("test_string_two");
    int expected = 0;

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    int actual = json_string_equals(json_array_get(obj, 0), json_compare_string.c_str());
    json_unref(obj);

    //Assert
    ASSERT_EQ(expected, actual);
}

TEST_F(json_tests, json_string_compare_with_non_string_returns_0)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("[1]");
    std::string json_compare_string("test_string_two");
    int expected = 0;

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    int actual = json_string_equals(json_array_get(obj, 0), json_compare_string.c_str());
    json_unref(obj);

    //Assert
    ASSERT_EQ(expected, actual);
}




