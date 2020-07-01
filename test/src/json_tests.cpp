#include <string>
#include <experimental/filesystem>
#include <zconf.h>
#include "gtest/gtest.h"
#include "test_file_utils.h"
#include "json.h"

#include "mem.h"

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

int isstring(json_object_t obj)
{
    return json_isstring(obj)
           || (json_isarray(obj)
               && json_array_length(obj) == 1
               && json_isstring(json_array_get(obj, 0)));
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
//    std::string filename("n_structure_open_array_object_small.json");
////    std::string filename("n_structure_unclosed_array.json");
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

