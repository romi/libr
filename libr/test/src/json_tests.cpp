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
        numcalls = 0;
    }

    void TearDown() override
    {
    }

public:
    int numcalls;

    void for_each_test(const char* key  __attribute__((unused)), json_object_t value  __attribute__((unused)))
    {
         numcalls++;
    }

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

TEST_F(json_tests, json_load_fails_when_nofile)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    std::string json_filename = "./nofile.zzd";

    // Act
    auto obj = json_load(json_filename.c_str(), &parse_error, error_message, sizeof(error_message));

    //Assert
    ASSERT_TRUE(json_isnull(obj));
    json_unref(obj);
}


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

TEST_F(json_tests, json_parse_parses_string)
{
    // Arrange
    std::string json_string("[100]");
    std::string json_compare_string("test_string_two");
    int expected = 100;

    // Act
    auto obj = json_parse(json_string.c_str());
    double actual = json_number_value(json_array_get(obj, 0));
    json_unref(obj);

    //Assert
    ASSERT_EQ(expected, actual);
}


TEST_F(json_tests, json_array_length_returns_correct_value)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("[1, 2, 3]");
    int expected = 3;

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    int actual = json_array_length(obj);
    json_unref(obj);

    //Assert
    ASSERT_EQ(expected, actual);
}

TEST_F(json_tests, json_array_length_incorrect_type_returns_0)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("\"test_string\"");
    int expected = 0;

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    int actual = json_array_length(obj);
    json_unref(obj);

    //Assert
    ASSERT_EQ(expected, actual);
}

TEST_F(json_tests, json_array_get_invalid_type_returns_json_null)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("\"test_string\"");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto actual = json_array_get(obj, 0);
    json_unref(obj);

    //Assert
    ASSERT_TRUE(json_isnull(actual));
}

TEST_F(json_tests, json_array_get_returns_correct_value)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("[7, 2, 3]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto array_obj = json_array_get(obj, 1);
    double actual = json_number_value(array_obj);
    json_unref(obj);

    //Assert
    ASSERT_EQ(actual, 2);
}

TEST_F(json_tests, json_array_get_invalid_index_returns_json_null)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int index = 20;
    std::string json_string("[7, 2, 3]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto array_obj = json_array_get(obj, index);
    json_unref(obj);

    //Assert
    ASSERT_TRUE(json_isnull(array_obj));
}

TEST_F(json_tests, json_array_getnum_invalid_type_returns_NAN)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("\"test_string\"");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto actual = json_array_getnum(obj, 0);
    json_unref(obj);

    //Assert
    ASSERT_TRUE(isnan(actual));
}

TEST_F(json_tests, json_array_getnum_returns_correct_value)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];

    std::string json_string("[7, 2, 3]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto actual = json_array_getnum(obj, 1);
    json_unref(obj);

    //Assert
    ASSERT_EQ(actual, 2);
}

TEST_F(json_tests, json_array_getnum_invalid_index_returns_NAN)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int index = 20;
    std::string json_string("[7, 2, 3]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto actual = json_array_getnum(obj, index);
    json_unref(obj);

    //Assert
    ASSERT_TRUE(isnan(actual));
}


TEST_F(json_tests, json_array_set_invalid_type_returns_0)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int index = 2;
    int numbervalue = 100;
    int expected = 0;

    std::string json_string("\"test_string\"");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto numberobj = json_number_create(numbervalue);
    auto actual = json_array_set(obj, numberobj, index);
    json_unref(obj);
    json_unref(numberobj);

    //Assert
    ASSERT_EQ(actual, expected);
}

TEST_F(json_tests, json_array_set_valid_index_returns_index)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int index = 2;
    int numbervalue = 100;
    int expected = index;

    std::string json_string("[7, 2, 3]");
    std::string json_expected_string("[7, 2, 100]");
    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto numberobj = json_number_create(numbervalue);
    auto actual = json_array_set(obj, numberobj, index);
    auto actual_number = json_array_getnum(obj, index);

    char buffer[256];

    json_tostring(obj, buffer, 256);
    std::string actual_string(buffer);

    json_unref(obj);
    json_unref(numberobj);

    // Assert
    ASSERT_EQ(actual, expected);
    ASSERT_EQ(actual_string, json_expected_string);
    ASSERT_EQ(actual_number, numbervalue);
}

TEST_F(json_tests, json_array_set_outofrange_index_returns_index_sets_value)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int index = 8;
    int numbervalue = 100;
    int expected = index;

    std::string json_string("[7, 2, 3]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto numberobj = json_number_create(numbervalue);
    auto actual = json_array_set(obj, numberobj, index);
    auto actual_number = json_array_getnum(obj, index);

    json_unref(obj);
    json_unref(numberobj);

    // Assert
    ASSERT_EQ(actual, expected);
    ASSERT_EQ(actual_number, numbervalue);
}

TEST_F(json_tests, json_array_set_negative_leaves_array_unchanged)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int index = -1;
    int numbervalue = 100;
    int expected = index;

    std::string json_string("[7, 2, 3]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto numberobj = json_number_create(numbervalue);
    auto actual = json_array_set(obj, numberobj, index);

    char buffer[256];
    json_tostring(obj, buffer, 256);
    std::string actual_string(buffer);

    json_unref(obj);
    json_unref(numberobj);

    // Assert
    ASSERT_EQ(actual, expected);
    ASSERT_EQ(json_string, actual_string);
}


TEST_F(json_tests, json_array_push_invalid_type_returns_0)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int numbervalue = 100;
    int expected = 0;

    std::string json_string("\"test_string\"");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto numberobj = json_number_create(numbervalue);
    auto actual = json_array_push(obj, numberobj);

    json_unref(obj);
    json_unref(numberobj);

    // Assert
    ASSERT_EQ(actual, expected);
}

TEST_F(json_tests, json_array_push_adds_value_to_end)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int numbervalue = 100;
    int expected_index = 3;

    std::string json_string("[7, 2, 3]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto numberobj = json_number_create(numbervalue);
    auto actual_index = json_array_push(obj, numberobj);
    auto actual_number = json_array_getnum(obj, actual_index);

    json_unref(obj);
    json_unref(numberobj);

    // Assert
    ASSERT_EQ(actual_index, expected_index);
    ASSERT_EQ(actual_number, numbervalue);
}

TEST_F(json_tests, json_setnum_sets_number)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int numbervalue = 100;
    int index = 3;

    std::string json_string("[7, 2, 3]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto numberobj = json_number_create(numbervalue);
    auto actual_index = json_array_setnum(obj, numbervalue, index);
    auto actual_number = json_array_getnum(obj, actual_index);

    json_unref(obj);
    json_unref(numberobj);

    // Assert
    ASSERT_EQ(actual_index, index);
    ASSERT_EQ(actual_number, numbervalue);
}

TEST_F(json_tests, json_setstr_sets_string)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    std::string string_value("hundred");
    int index = 2;

    std::string json_string("[7, 2, 3]");
    std::string json_expected_string("[7, 2, \"hundred\"]");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto actual_index = json_array_setstr(obj, string_value.c_str(), index);
    auto actual_chars = json_array_getstr(obj, actual_index);
    std::string added_chars(actual_chars);

    char buffer[256];
    json_tostring(obj, buffer, 256);
    std::string actual_string(buffer);

    json_unref(obj);

    // Assert
    ASSERT_EQ(actual_index, index);
    ASSERT_EQ(actual_string, json_expected_string);
    ASSERT_EQ(added_chars, string_value);
}

// Address sanitizer test.
TEST_F(json_tests, json_object_create_and_unref_cleans_object)
{
    // Arrange
    json_object_t obj = json_object_create();

    // Act
    json_unref(obj);

    // Assert
}

TEST_F(json_tests, json_object_set_wrong_type_returns_minus_1)
{
    // Arrange
    int expected = -1;
    //   json_object_t obj = json_object_create();
    auto numberobj = json_number_create(10);

    // Act
    int actual = json_object_set(numberobj, "key", numberobj);

    // Assert
    ASSERT_EQ(actual, expected);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_has_wrong_type_returns_0)
{
    // Arrange
    int expected = 0;
 //   json_object_t obj = json_object_create();
    auto numberobj = json_number_create(10);

    // Act
    int actual = json_object_has(numberobj, "key");

    // Assert
    ASSERT_EQ(actual, expected);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_has_object_doesnt_exist_returns_0)
{
    // Arrange
    int expected = 0;
    json_object_t obj = json_object_create();
   // auto numberobj = json_number_create(10);

    // Act
    int actual = json_object_has(obj, "key");

    // Assert
    ASSERT_EQ(actual, expected);
    json_unref(obj);
}

TEST_F(json_tests, json_object_has_object_exists_returns_1)
{
    // Arrange
    int expected = 1;
    std::string key("key");

    json_object_t obj = json_object_create();
     auto numberobj = json_number_create(10);
     json_object_set(obj, key.c_str(), numberobj);

    // Act
    int actual = json_object_has(obj, key.c_str());

    // Assert
    ASSERT_EQ(actual, expected);
    json_unref(obj);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_get_object_wrong_type_returns_json_null)
{
    // Arrange
    std::string key("key");

    auto numberobj = json_number_create(10);

    // Act
    json_object_t actual = json_object_get(numberobj, key.c_str());

    // Assert
    ASSERT_TRUE(json_isnull(actual));
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_get_object_doesnt_exist_returns_json_undefined)
{
    // Arrange
    std::string key("key");

    json_object_t obj = json_object_create();

    // Act
    json_object_t actual = json_object_get(obj, key.c_str());

    // Assert
    ASSERT_TRUE(json_isundefined(actual));
    json_unref(obj);
}

TEST_F(json_tests, json_object_get_object_exists_returns_object)
{
    // Arrange
    std::string key("key");
    double value = 10;

    json_object_t obj = json_object_create();
    auto numberobj = json_number_create(value);
    json_object_set(obj, key.c_str(), numberobj);

    // Act
    json_object_t actual = json_object_get(obj, key.c_str());
    double actual_number = json_number_value(numberobj);
    // Assert
    ASSERT_TRUE(json_isnumber(actual));
    ASSERT_EQ(value, actual_number);
    json_unref(obj);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_getnum_object_does_not_exist_returns_NAN)
{
    // Arrange
    std::string key("key");

    json_object_t obj = json_object_create();

    // Act
    double actual = json_object_getnum(obj, key.c_str());
    // Assert
    ASSERT_TRUE(isnan(actual));
    json_unref(obj);
}

TEST_F(json_tests, json_object_getnum_object_exists_returns_number)
{
    // Arrange
    std::string key("key");
    double value = 10;

    json_object_t obj = json_object_create();
    auto numberobj = json_number_create(value);
    json_object_set(obj, key.c_str(), numberobj);

    // Act
    double actual = json_object_getnum(obj, key.c_str());
    // Assert
    ASSERT_EQ(value, actual);
    json_unref(obj);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_getbool_object_does_not_exist_returns_error)
{
    // Arrange
    std::string key("key");
    int expected = -1;

    json_object_t obj = json_object_create();

    // Act
    int actual = json_object_getbool(obj, key.c_str());
    // Assert
    ASSERT_EQ(expected, actual);
    json_unref(obj);
}

TEST_F(json_tests, json_object_getbool_object_true_returns_1)
{
    // Arrange
    std::string key("key");
    int expected = 1;

    json_object_t obj = json_object_create();
    json_object_setbool(obj, key.c_str(), true);

    // Act
    int actual = json_object_getbool(obj, key.c_str());
    // Assert
    ASSERT_EQ(expected, actual);
    json_unref(obj);
}

TEST_F(json_tests, json_object_getbool_object_false_returns_0)
{
    // Arrange
    std::string key("key");
    int expected = 0;

    json_object_t obj = json_object_create();
    json_object_setbool(obj, key.c_str(), false);

    // Act
    int actual = json_object_getbool(obj, key.c_str());
    // Assert
    ASSERT_EQ(expected, actual);
    json_unref(obj);
}

TEST_F(json_tests, json_object_getstr_object_does_not_exist_returns_NULL)
{
    // Arrange
    std::string key("key");

    json_object_t obj = json_object_create();

    // Act
    const char* actual = json_object_getstr(obj, key.c_str());

    // Assert
    ASSERT_EQ(actual, nullptr);
    json_unref(obj);
}

TEST_F(json_tests, json_object_getstr_object_wrong_type_returns_NULL)
{
    // Arrange
    std::string key("key");
    int value = 10;

    json_object_t obj = json_object_create();
    auto numberobj = json_number_create(value);
    json_object_set(obj, key.c_str(), numberobj);

    // Act
    const char* actual = json_object_getstr(obj, key.c_str());

    // Assert
    ASSERT_EQ(actual, nullptr);
    json_unref(obj);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_getstr_object_correct_returns_string)
{
    // Arrange
    std::string key("key");
    std::string value("value");

    json_object_t obj = json_object_create();
    json_object_setstr(obj, key.c_str(), value.c_str());

    // Act
    const char* actual = json_object_getstr(obj, key.c_str());

    // Assert
    ASSERT_EQ(std::string(actual), value);
    json_unref(obj);
}

TEST_F(json_tests, json_object_unset_no_object_returs_error)
{
    // Arrange
    std::string key("key");
    std::string value("value");
    int expected = -1;

    json_object_t obj = json_object_create();

    // Act
    int actual = json_object_unset(obj, key.c_str());

    const char* actual_string = json_object_getstr(obj, key.c_str());

    // Assert
    ASSERT_EQ(actual_string, nullptr);
    ASSERT_EQ(actual, expected);
    json_unref(obj);
}

TEST_F(json_tests, json_object_unset_wrong_object_type_returs_error)
{
    // Arrange
    std::string key("key");
    int expected = -1;
    int value = 10;

    auto numberobj = json_number_create(value);

    // Act
    int actual = json_object_unset(numberobj, key.c_str());

    // Assert
    ASSERT_EQ(actual, expected);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_unset_object_returs_0)
{
    // Arrange
    std::string key("key");
    std::string value("value");
    int expected = 0;

    json_object_t obj = json_object_create();
    json_object_setstr(obj, key.c_str(), value.c_str());

    // Act
    int actual = json_object_unset(obj, key.c_str());

    const char* actual_string = json_object_getstr(obj, key.c_str());

    // Assert
    ASSERT_EQ(actual_string, nullptr);
    ASSERT_EQ(actual, expected);
    json_unref(obj);
}

static int32_t for_each_function_call(const char* key, json_object_t value, void *data)
{
    json_tests* tests = (json_tests*)data;
    tests->for_each_test(key, value);
    return 0;
}

TEST_F(json_tests, json_object_for_each_no_objects_not_called)
{
    // Arrange
    std::string key("key");
    std::string value("value");
    int expected = 0;

    json_object_t obj = json_object_create();

    // Act
    int actual = json_object_foreach(obj, for_each_function_call, this);

    // Assert
    ASSERT_EQ(actual, expected);
    ASSERT_EQ(numcalls, 0);
    json_unref(obj);
}

TEST_F(json_tests, json_object_for_each_wrong_object_type_not_called)
{
    // Arrange
    std::string key("key");
    int value = 10;
    int expected = -1;
    auto numberobj = json_number_create(value);

    // Act
    int actual = json_object_foreach(numberobj, for_each_function_call, this);

    // Assert
    ASSERT_EQ(actual, expected);
    ASSERT_EQ(numcalls, 0);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_for_each_3_objects_called_3_times)
{
    // Arrange
    std::string key10("key10");
    std::string key11("key11");
    std::string key12("key12");
    int value = 10;
    int expected = 3;

    json_object_t obj = json_object_create();

    json_object_setnum(obj, key10.c_str(), value++);
    json_object_setnum(obj, key11.c_str(), value++);
    json_object_setnum(obj, key12.c_str(), value++);

    // Act
    int actual = json_object_foreach(obj, for_each_function_call, this);

    // Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(numcalls, expected);
    json_unref(obj);
}

TEST_F(json_tests, json_object_length_is_correct_empty)
{
    // Arrange
    int expected_size = 0;
    json_object_t obj = json_object_create();

    // Act
    int actual = json_object_length(obj);

    // Assert
    ASSERT_EQ(actual, expected_size);
    json_unref(obj);
}

TEST_F(json_tests, json_object_length_wrong_object_type_is_correct)
{
    // Arrange
    int expected_size = 0;
    int value = 10;
    auto numberobj = json_number_create(value);

    // Act
    int actual = json_object_length(numberobj);

    // Assert
    ASSERT_EQ(actual, expected_size);
    json_unref(numberobj);
}

TEST_F(json_tests, json_object_length_is_correct)
{
    // Arrange
    int expected_size = 3;
    std::string key10("key10");
    std::string key11("key11");
    std::string key12("key12");

    int value = 10;

    json_object_t obj = json_object_create();

    json_object_setnum(obj, key10.c_str(), value++);
    json_object_setnum(obj, key11.c_str(), value++);
    json_object_setnum(obj, key12.c_str(), value++);

    // Act
    int actual = json_object_length(obj);

    // Assert
    ASSERT_EQ(actual, expected_size);
    json_unref(obj);
}

TEST_F(json_tests, json_object_length_hashtable_resizes_hashtable)
{
    // Arrange
    std::string key("key");
    int number_of_values = 24;

    // Act
    json_object_t obj = json_object_create();
    int length = json_object_length(obj);

    for (int value = 0; value < number_of_values; value++)
        json_object_setnum(obj, std::string(key + std::to_string(value)).c_str(), value);

    int actual_length = json_object_length(obj);

    json_unref(obj);

    // Assert
    ASSERT_EQ(length, 0);
    ASSERT_EQ(actual_length, number_of_values);
}

TEST_F(json_tests, json_tofile_writes_file)
{
    // Arrange
    std::string key10("key10");
    std::string key11("key11");
    std::string key12("key12");
    int value = 10;
    std::string filename = "./file.json";

    std::string expected_json_string("{\"key10\":10,\"key11\":11,\"key12\":\"12\"}");

    // Act
    json_object_t obj = json_object_create();

    json_object_setnum(obj, key10.c_str(), value++);
    json_object_setnum(obj, key11.c_str(), value++);
    json_object_setstr(obj, key12.c_str(), std::to_string(value++).c_str());

    int result = json_tofile(obj, 0, filename.c_str());

    std::string actual_json_string = ReadFileAsString(filename);
    fs::remove(filename);

    json_unref(obj);

    // Assert
    ASSERT_EQ(result, 0);
    ASSERT_EQ(actual_json_string, expected_json_string);
}

TEST_F(json_tests, json_tofile_fails_returns_error)
{
    // Arrange
    std::string key10("key10");
    std::string key11("key11");
    std::string key12("key12");
    int value = 10;
    std::string filename = "/";

    // Act
    json_object_t obj = json_object_create();

    json_object_setnum(obj, key10.c_str(), value++);
    json_object_setnum(obj, key11.c_str(), value++);
    json_object_setnum(obj, key12.c_str(), value++);

    int result = json_tofile(obj, 0, filename.c_str());

    json_unref(obj);
    // Assert
    ASSERT_EQ(result, -1);
}

TEST_F(json_tests, json_tofile_pretty_writes_pretty_file)
{
    // Arrange
    std::string key10("key10");
    std::string key11("key11");
    std::string key12("key12");
    int value = 10;
    std::string filename = "./file.json";

    std::string expected_json_string("{\"key10\":10,\"key11\":11,\"key12\":12}");

    // Act
    json_object_t obj = json_object_create();

    json_object_setnum(obj, key10.c_str(), value++);
    json_object_setnum(obj, key11.c_str(), value++);
    json_object_setnum(obj, key12.c_str(), value++);

    int result = json_tofile(obj, k_json_pretty, filename.c_str());

    std::string actual_json_string = ReadFileAsString(filename);


    json_unref(obj);
    fs::remove(filename);
    // Assert
    ASSERT_EQ(result, 0);
}

TEST_F(json_tests, json_print_prints_to_stdout)
{
    // Arrange
    std::string key10("key10");
    std::string key11("key11");
    std::string key12("key12");
    int value = 10;
    std::string expected_json_string("{\"key10\":10,\"key11\":11,\"key12\":\"12\"}\n");

    // Act
    json_object_t obj = json_object_create();

    json_object_setnum(obj, key10.c_str(), value++);
    json_object_setnum(obj, key11.c_str(), value++);
    json_object_setstr(obj, key12.c_str(), std::to_string(value++).c_str());

    testing::internal::CaptureStdout();
    json_print(obj, 0);
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    json_unref(obj);

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_json_string);
}

TEST_F(json_tests, json_print_serialises_null_and_undefined_types)
{
    // Arrange
    std::string key10("key10");
    std::string key11("key11");
    std::string key12("key12");

    std::string expected_json_string("{\"key10\":null,\"key11\":undefined}\n");

    // Act
    json_object_t obj = json_object_create();

    json_object_set(obj, key10.c_str(), json_null());
    json_object_set(obj, key11.c_str(), json_undefined());

    testing::internal::CaptureStdout();
    json_print(obj, 0);
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    json_unref(obj);

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_json_string);
}

TEST_F(json_tests, json_print_serialises_float_number_types)
{
    // Arrange
    std::string key10("key10");
    float value = 200.192836234;

    std::string expected_json_string("{\"key10\":200.192841}\n");

    // Act
    json_object_t obj = json_object_create();

    json_object_setnum(obj, key10.c_str(), value);

    testing::internal::CaptureStdout();
    json_print(obj, 0);
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    json_unref(obj);

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_json_string);
}

TEST_F(json_tests, json_print_array_prints_nulls)
{
    // Arrange
    int parse_error = 0;
    char error_message[256];
    int index = 128;
    int numbervalue = 100;
    int expected = index;

    std::string json_string("[7, 2, 3]");
    std::string expected_json_string("[7, 2, 3, null, null, null, null, null, null, null, null, null, null, null, null, null");

    // Act
    auto obj = json_parse_ext(json_string.c_str(), &parse_error, error_message, sizeof(error_message));
    auto numberobj = json_number_create(numbervalue);
    auto actual = json_array_set(obj, numberobj, index);
    auto actual_number = json_array_getnum(obj, index);

    testing::internal::CaptureStdout();
    json_print(obj, k_json_pretty);
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    size_t contains = actual_stdoutput.find(expected_json_string);

    json_unref(obj);
    json_unref(numberobj);

    // Assert
    ASSERT_EQ(actual, expected);
    ASSERT_EQ(actual_number, numbervalue);
    ASSERT_NE(contains, std::string::npos);
}

TEST_F(json_tests, json_print_object_pretty_indents_objects_COVERAGE)
{
    // Arrange
        int number_of_values = 8;
        std::string key = "key";

        // Act
        json_object_t obj1 = json_object_create();
        json_object_t obj2 = json_object_create();

        for (int value = 0; value < number_of_values; value++)
            json_object_setnum(obj2, std::string(key + std::to_string(value)).c_str(), value);

        json_object_setnum(obj1, "obj1val", 10);
        json_object_set(obj1, "obj2val", obj2);
        json_object_setstr(obj1, "stringkey", std::to_string(20).c_str());

        testing::internal::CaptureStdout();
        json_print(obj1, 0);
        std::string actual_stdoutput = testing::internal::GetCapturedStdout();

        json_unref(obj1);
        json_unref(obj2);
        // Assert
        //ToDo: Need a test.

}

// ToDo: Why does this break address sanitizer?
//TEST_F(json_tests, json_cleanup_COVERAGE)
//{
//    // Arrange
//
//    // Act
//    json_cleanup();
//    // Assert
//}

