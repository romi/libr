#include <string>
#include <fstream>
#include <zconf.h>
#include "gtest/gtest.h"

#include "fs.h"

extern "C" {
#include "log.mock.h"
#include "os_wrapper.mock.h"
}

std::string existing_file("exists.txt");
std::string non_existing_file("doesnt_exist.txt");

std::string directory_name("directory1");

class fs_posix_tests : public ::testing::Test
{
protected:
    fs_posix_tests() = default;

    ~fs_posix_tests() override = default;

    void SetUp() override
    {
        RESET_FAKE(stat_wrapper);
    }

    void TearDown() override
    {
    }

    void SetupFiles()
    {
        std::ofstream outFile(existing_file, std::ios::trunc);
        outFile.exceptions(std::ios::badbit | std::ios::failbit);
        outFile << "Humpty Dumpty sat on a wall.\n";
        outFile << "Humpty Dumpty had a great fall,\n";

        mkdir(directory_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    static int stat_wrapper_custom_fake (const char * __file, struct stat * __buf)
    {
        memcpy(__buf, &stat_wrapper_stat_buffer, sizeof(struct stat));
    }

    static int stat_wrapper_return_value;
    static struct stat stat_wrapper_stat_buffer;

};

int    fs_posix_tests::stat_wrapper_return_value;
struct stat fs_posix_tests::stat_wrapper_stat_buffer;


TEST_F(fs_posix_tests, path_exists_when_null_path_returns_0)
{
    // Arrange
    // Call the real stat for null behaviour
    stat_wrapper_fake.custom_fake = stat;

    // Act
     int actual = path_exists(nullptr);

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, path_exists_when_path_does_not_exist_returns_0)
{
    // Arrange
    const char *path = "/random/path";
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = path_exists(path);

    //Assert
    ASSERT_EQ(actual, 0);
}
TEST_F(fs_posix_tests, path_exists_when_path_exists_returns_1)
{
    // Arrange
    const char *path = "/";
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = path_exists(path);

    //Assert
    ASSERT_EQ(actual, 1);
}

TEST_F(fs_posix_tests, is_file_when_file_doesnt_exist_returns_0)
{
    // Arrange
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = is_file(non_existing_file.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, is_file_when_file_exists_returns_1)
{
    // Arrange
    SetupFiles();
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = is_file(existing_file.c_str());

    //Assert
    ASSERT_EQ(actual, 1);
}

TEST_F(fs_posix_tests, is_file_when_file_is_dir_returns_0)
{
    // Arrange
    SetupFiles();
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = is_file(directory_name.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, is_directory_when_directory_doesnt_exist_returns_0)
{
    // Arrange
    std::string non_existent_directory("nothing_here");
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = is_directory(non_existent_directory.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, is_directory_when_directory_exists_returns_1)
{
    // Arrange
    SetupFiles();
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = is_directory(directory_name.c_str());

    //Assert
    ASSERT_EQ(actual, 1);
}

TEST_F(fs_posix_tests, is_directory_when_directory_is_file_returns_0)
{
    // Arrange
    SetupFiles();
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = is_directory(existing_file.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, path_is_absolute_when_absolute_returns_not_0)
{
    // Arrange
    std::string abspath("/abspath");
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = path_is_absolute(abspath.c_str());

    //Assert
    ASSERT_NE(actual, 0);
}

TEST_F(fs_posix_tests, path_is_absolute_when_not_absolute_returns_0)
{
    // Arrange
    std::string abspath("path");
    stat_wrapper_fake.custom_fake = stat;

    // Act
    int actual = path_is_absolute(abspath.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, path_make_absolute_absolute_returns_input)
{
    // Arrange
    std::string abspath("/path");
    stat_wrapper_fake.custom_fake = stat;

    char buffer[PATH_MAX];;

    // Act
    int actual = path_make_absolute(abspath.c_str(), buffer, (int)PATH_MAX);

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, path_make_absolute_not_absolute_returns_correct_path)
{
    // Arrange
    std::string abspath("path");
    stat_wrapper_fake.custom_fake = stat;

    char buffer[PATH_MAX];;

    char * dname = get_current_dir_name();
    char * rpath = realpath(existing_file.c_str(), NULL);

    // Act
    int actual = path_make_absolute(abspath.c_str(), buffer, (int)PATH_MAX);

    //Assert
    ASSERT_EQ(actual, 0);
    free(dname);
}