#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <zconf.h>
#include "gtest/gtest.h"
#include "test_file_utils.h"

#include "fs.h"
#include "mem.h"

extern "C" {
#include "log.mock.h"
#include "os_wrapper.mock.h"
#include "mem.mock.h"
}

namespace fs = std::experimental::filesystem;

class fs_posix_tests : public ::testing::Test
{
protected:
    fs_posix_tests() = default;

    ~fs_posix_tests() override = default;

    void TestBreak()
    {
        std::vector<std::string> paths;
        PathBreak(paths);

        for(auto& element : paths)
            std::cout << element << std::endl;

    }

    void PathBreak(std::vector<std::string>& paths)
    {
        fs::path pathstring = "/users/abcdef/AppData/Local/Temp";
        for(auto& element : pathstring)
            paths.emplace_back(element);
    }


    void SetupFiles()
    {
        if (!is_directory(testDirectory.c_str()))
        {
            mkdir(testDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

            MakeFile(file1Path, filePath1Data);
            MakeFile(file2Path, filePath2Data);
        }
    }

    void SetUp() override
    {
        newDirectory = "";
        RESET_FAKE(r_err);
    }

    void TearDown() override
    {
        if (fs::is_directory(fs::path(testDirectory)))
        {
            fs::remove_all(testDirectory.c_str());
        }
        if (fs::is_directory(fs::path(newDirectory)))
        {
            fs::remove_all(newDirectory.c_str());
        }
    }

    const std::string testDirectory = "./testDirectory/";
    const std::string file1Path = testDirectory + std::string("filepath1");
    const std::string file2Path = testDirectory + std::string("filepath2");
    const std::string non_existing_file = "doesnt_exist.txt";
    const std::string non_existing_path = "/somepath/doesnt_exist/";
    std::string filePath1Data = "Humpty Dumpty sat on a wall";
    std::string filePath2Data =  "Humpty Dumpty had a great fall";
    std::string newDirectory = "./newDirectory/";

};

TEST_F(fs_posix_tests, path_exists_when_null_path_returns_0)
{
    // Arrange
    // Act
     int actual = path_exists(nullptr);

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, path_exists_when_path_does_not_exist_returns_0)
{
    // Arrange
    // Act
    int actual = path_exists(non_existing_file.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}
TEST_F(fs_posix_tests, path_exists_when_path_exists_returns_1)
{
    // Arrange
    SetupFiles();

    // Act
    int actual = path_exists(testDirectory.c_str());

    //Assert
    ASSERT_EQ(actual, 1);
}

TEST_F(fs_posix_tests, is_file_when_file_null_returns_0)
{
    // Arrange
    // Act
    int actual = is_file(nullptr);

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, is_file_when_file_doesnt_exist_returns_0)
{
    // Arrange
    // Act
    int actual = is_file(non_existing_file.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, is_file_when_file_exists_returns_1)
{
    // Arrange
    SetupFiles();

    // Act
    int actual = is_file(file1Path.c_str());

    //Assert
    ASSERT_EQ(actual, 1);
}

TEST_F(fs_posix_tests, is_file_when_file_is_dir_returns_0)
{
    // Arrange
    SetupFiles();

    // Act
    int actual = is_file(testDirectory.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, is_directory_when_directory_doesnt_exist_returns_0)
{
    // Arrange
    // Act
    int actual = is_directory(non_existing_path.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, is_directory_when_directory_exists_returns_1)
{
    // Arrange
    SetupFiles();

    // Act
    int actual = is_directory(testDirectory.c_str());

    //Assert
    ASSERT_EQ(actual, 1);
}

TEST_F(fs_posix_tests, is_directory_when_directory_is_file_returns_0)
{
    // Arrange
    SetupFiles();

    // Act
    int actual = is_directory(file1Path.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, path_is_absolute_when_absolute_returns_not_0)
{
    // Arrange
    std::string abspath("/abspath");

    // Act
    int actual = path_is_absolute(abspath.c_str());

    //Assert
    ASSERT_NE(actual, 0);
}

TEST_F(fs_posix_tests, path_is_absolute_when_not_absolute_returns_0)
{
    // Arrange
    std::string abspath("path");

    // Act
    int actual = path_is_absolute(abspath.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, path_make_absolute_absolute_returns_input)
{
    // Arrange
    std::string abspath("/path");
    char buffer[PATH_MAX];

    // Act
    int actual = path_make_absolute(abspath.c_str(), buffer, (int)PATH_MAX);
    std::string bufferstring(buffer);
    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(bufferstring, abspath);
}

TEST_F(fs_posix_tests, path_make_absolute_not_absolute_returns_correct_path)
{
    // Arrange
    std::string expected_absolute_path = "";
    std::string abspath("path");

    char buffer[PATH_MAX];;

    char * dname = get_current_dir_name();
    expected_absolute_path = dname;
    expected_absolute_path += '/' + abspath;

    // Act
    int actual = path_make_absolute(abspath.c_str(), buffer, (int)PATH_MAX);
    std::string actual_absolute_path(buffer);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(actual_absolute_path, expected_absolute_path);
    free(dname);
}

TEST_F(fs_posix_tests, path_break_with_null_returns_null)
{
    // Arrange
    // Act
    list_t *actual = path_break(nullptr);

    //Assert
    ASSERT_EQ(actual, nullptr);
}

TEST_F(fs_posix_tests, path_break_with_root_returns_correct_entry)
{
    // Arrange
    std::string root_path = "/";

    // Act
    list_t *actual = path_break(root_path.c_str());
    std::string actual_string((char*)actual->data);
    //Assert
    ASSERT_EQ(list_size(actual), 1);
    ASSERT_EQ(actual_string, root_path);
    path_delete(actual);
}

TEST_F(fs_posix_tests, path_break_with_root_subdirectory_returns_correct_entry)
{
    // Arrange
    std::string root_path = "/";
    std::string path = "testdirectory";
    std::string fullpath = root_path + path;

    // Act
    list_t *actual = path_break(fullpath.c_str());
    std::string actual_root_string((char*)actual->data);
    std::string actual_path_string((char*)actual->next->data);

    //Assert
    ASSERT_EQ(list_size(actual), 2);
    ASSERT_EQ(actual_root_string, root_path);
    ASSERT_EQ(actual_path_string, path);
    path_delete(actual);
}

TEST_F(fs_posix_tests, path_break_with_single_part_returns_correct_list)
{
    // Arrange
    std::string path = "testpath";

    // Act
    list_t *actual = path_break(path.c_str());
    std::string actual_string((char*)actual->data);
    //Assert
    ASSERT_EQ(list_size(actual), 1);
    ASSERT_EQ(actual_string, path);
    path_delete(actual);
}

TEST_F(fs_posix_tests, path_break_with_multiple_parts_returns_correct_list)
{
    // Arrange
    std::string path = "testpath";
    std::string fullpath = path + '/' + path + '/' + path;

    // Act
    list_t *actual = path_break(fullpath.c_str());
    std::string actual_string((char*)actual->data);
    int listsize = list_size(actual);

    list_t * entry0 = list_nth(actual, 0);
    list_t * entry1 = list_nth(actual, 1);
    list_t * entry2 = list_nth(actual, 2);

    //Assert
    ASSERT_EQ(listsize, 3);
    ASSERT_EQ(std::string((char*)entry0->data), path);
    ASSERT_EQ(std::string((char*)entry1->data), path);
    ASSERT_EQ(std::string((char*)entry2->data), path);
    path_delete(actual);
}

TEST_F(fs_posix_tests, path_break_root_with_multiple_parts_returns_correct_list)
{
    // Arrange
    std::string root_path = "/";
    std::string path = "testpath";
    std::string fullpath = root_path + path + '/' + path + '/' + path;

    // Act
    list_t *actual = path_break(fullpath.c_str());
    std::string actual_string((char*)actual->data);
    int listsize = list_size(actual);

    list_t * entry0 = list_nth(actual, 0);
    list_t * entry1 = list_nth(actual, 1);
    list_t * entry2 = list_nth(actual, 2);
    list_t * entry3 = list_nth(actual, 3);

    //Assert
    ASSERT_EQ(listsize, 4);
    ASSERT_EQ(std::string((char*)entry0->data), root_path);
    ASSERT_EQ(std::string((char*)entry1->data), path);
    ASSERT_EQ(std::string((char*)entry2->data), path);
    ASSERT_EQ(std::string((char*)entry3->data), path);
    path_delete(actual);
}

TEST_F(fs_posix_tests, path_break_root_with_multiple_parts_slash_terminated_returns_correct_list)
{
    // Arrange
    std::string root_path = "/";
    std::string path = "testpath";
    std::string fullpath = root_path + path + '/' + path + '/' + path + '/';

    // Act
    list_t *actual = path_break(fullpath.c_str());
    std::string actual_string((char*)actual->data);
    int listsize = list_size(actual);

    list_t * entry0 = list_nth(actual, 0);
    list_t * entry1 = list_nth(actual, 1);
    list_t * entry2 = list_nth(actual, 2);
    list_t * entry3 = list_nth(actual, 3);

    //Assert
    ASSERT_EQ(listsize, 4);
    ASSERT_EQ(std::string((char*)entry0->data), root_path);
    ASSERT_EQ(std::string((char*)entry1->data), path);
    ASSERT_EQ(std::string((char*)entry2->data), path);
    ASSERT_EQ(std::string((char*)entry3->data), path);

    path_delete(actual);
}

TEST_F(fs_posix_tests, path_glue_root_with_null_list_returns_error)
{
    // Arrange
    char buffer[10];
    // Act
    int actual = path_glue(nullptr, 0, buffer, 0);

    //Assert
    ASSERT_EQ(actual, -1);
}

TEST_F(fs_posix_tests, path_glue_root_with_null_buffer_returns_error)
{
    // Arrange
    list_t *elements = new_list(nullptr);

    // Act
    int actual = path_glue(elements, 0, nullptr, 0);

    //Assert
    ASSERT_EQ(actual, -1);

    delete_list(elements);
}

TEST_F(fs_posix_tests, path_glue_root_path_valid_buffer_size_0_returns_error)
{
    // Arrange
    char buffer[4] = {0,0,0,0};
    char root[] = "/";

    _list_t listelement1 = {root, nullptr};

    // Act
    int actual = path_glue(&listelement1, 0, buffer, 0);

    //Assert
    ASSERT_EQ(actual, -1);
}

TEST_F(fs_posix_tests, path_glue_root_path_valid_buffer_size_returns_correct_buffer)
{
    // Arrange
    const int buffer_size = 4;
    char buffer[buffer_size] = {0,0,0,0};
    char root[] = "/";

    _list_t listelement1 = {root, nullptr};

    // Act
    int actual = path_glue(&listelement1, 0, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(std::string(buffer), std::string(root));
}

TEST_F(fs_posix_tests, path_glue_path_root_valid_buffer_size_returns_correct_buffer)
{
    // Arrange
    const int buffer_size = 512;
    char buffer[buffer_size];
    char root[] = "/";
    std::string path1 = "path1";
    std::string path2 = "path2";
    std::string path3 = "path3";

    std::string expected_path = root + path1 + root + path2 + root + path3;

    _list_t listelement3 = {&path3[0], nullptr};
    _list_t listelement2 = {&path2[0], &listelement3};
    _list_t listelement1 = {&path1[0], &listelement2};
    _list_t listelement0 = {root, &listelement1};

    // Act
    int actual = path_glue(&listelement0, 0, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(std::string(buffer), std::string(expected_path));
}

TEST_F(fs_posix_tests, path_glue_path_not_root_valid_buffer_size_returns_correct_buffer)
{
    // Arrange
    const int buffer_size = 512;
    char buffer[buffer_size];
    char slash[] = "/";
    std::string path1 = "path1";
    std::string path2 = "path2";
    std::string path3 = "path3";

    std::string expected_path = path1 + slash + path2 + slash + path3;

    _list_t listelement2 = {&path3[0], nullptr};
    _list_t listelement1 = {&path2[0], &listelement2};
    _list_t listelement0 = {&path1[0], &listelement1};

    // Act
    int actual = path_glue(&listelement0, 0, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(std::string(buffer), std::string(expected_path));
}

TEST_F(fs_posix_tests, path_glue_path_not_root_is_absolute_valid_buffer_size_returns_correct_buffer)
{
    // Arrange
    const int buffer_size = 512;
    char buffer[buffer_size];
    char slash[] = "/";
    std::string path1 = "path1";
    std::string path2 = "path2";
    std::string path3 = "path3";

    std::string expected_path = slash + path1 + slash + path2 + slash + path3;

    _list_t listelement2 = {&path3[0], nullptr};
    _list_t listelement1 = {&path2[0], &listelement2};
    _list_t listelement0 = {&path1[0], &listelement1};

    // Act
    int actual = path_glue(&listelement0, 1, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(std::string(buffer), std::string(expected_path));
}

// Address sanitizer would fail this test if the memory was double deleted.
// we arn't mocking list in this set of tests.
TEST_F(fs_posix_tests, path_delete_with_empth_list_does_not_delete)
{
    // Arrange
    _list_t *listelement0 = new_list(nullptr);
    // Act
    path_delete(listelement0);
    //Assert
}

TEST_F(fs_posix_tests, directory_list_when_directory_null_returns_null)
{
    // Arrange
    // Act
    list_t *dirlisting = directory_list(nullptr);

    //Assert
    ASSERT_EQ(dirlisting, nullptr);
}

TEST_F(fs_posix_tests, directory_list_when_directory_not_exist_returns_null)
{
    // Arrange
    // Act
    list_t *dirlisting = directory_list(non_existing_path.c_str());

    //Assert
    ASSERT_EQ(dirlisting, nullptr);
}

TEST_F(fs_posix_tests, directory_list_when_directory_exists_returns_files)
{
    // Arrange
    SetupFiles();

    // Act
    list_t *dirlisting = directory_list(testDirectory.c_str());

    //Assert
    ASSERT_EQ(list_size(dirlisting), 2);
    delete_list_and_data(dirlisting, NULL);
}

TEST_F(fs_posix_tests, directory_create_when_directory_exists_returns_0)
{
    // Arrange
    SetupFiles();

    // Act
    int actual = directory_create(testDirectory.c_str());

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(fs_posix_tests, directory_create_when_path_exists_as_file_logs_err_returns_error)
{
    // Arrange
    SetupFiles();

    // Act
    int actual = directory_create(file1Path.c_str());

    //Assert
    ASSERT_EQ(actual, -1);
    ASSERT_EQ(r_err_fake.call_count, 1);
}

TEST_F(fs_posix_tests, directory_create_when_single_path_no_slash_doesnt_exist_creates_directory)
{
    // Arrange
    SetupFiles();
    newDirectory = "./newDirectory";

    // Act
    int actual = directory_create(newDirectory.c_str());
    int isdir = is_directory(newDirectory.c_str());
    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(isdir, 1);
}

TEST_F(fs_posix_tests, directory_create_when_single_path_with_slash_doesnt_exist_creates_directory)
{
    // Arrange
    newDirectory = "./newDirectory/";

    // Act
    int actual = directory_create(newDirectory.c_str());
    int isdir = is_directory(newDirectory.c_str());
    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(isdir, 1);
}

TEST_F(fs_posix_tests, directory_create_when_mkdir_fails_returns_error)
{
    // Arrange
    newDirectory = "./newDirectory/";
    std::string subdir =  newDirectory  + "subdir";
    // readonly
    mkdir(newDirectory.c_str(), S_IRUSR | S_IRGRP | S_IROTH | S_IXOTH);

    // Act
    int actual = directory_create(subdir.c_str());
    //Assert
    ASSERT_EQ(actual, -1);
}

TEST_F(fs_posix_tests, path_chown_user_doesnot_exist_fails)
{
    // Arrange
    const char *unknownuser = "thisuserdoesntexit01";

    // Act
    int actual = path_chown(testDirectory.c_str(), unknownuser);

    //Assert
    ASSERT_EQ(actual, -1);
    ASSERT_EQ(r_err_fake.call_count, 2);
}

// This test fails on Github actions with error code 6
// ENXIO
//The calling process has no controlling terminal. 
TEST_F(fs_posix_tests, path_chown_path_doesnot_exist_fails)
{
    // Arrange
    char knownusername[128] = {0};
    std::string unknownpath = "/somerandompaththatdoesntexist";

    int nGet = getlogin_r(knownusername, sizeof(knownusername)-1);

    int actual = 0;
    if (nGet == 0)
    {
        // Act
        actual = path_chown(unknownpath.c_str(), knownusername);
    }
    else
    {
        std::cout << "could not get current user" <<  std::endl;
        actual = -1;
    }

    //Assert
    ASSERT_EQ(actual, -1);
    ASSERT_EQ(r_err_fake.call_count, 0);
}

TEST_F(fs_posix_tests, file_store_invalid_path_fails)
{
    // Arrange
    const int datalen = 5;
    char data[datalen] = "data";

    std::string unknownpath = "/somerandompaththatdoesntexist/testfile";

    // Act
    int actual = file_store(unknownpath.c_str(), data, datalen, 0);

    //Assert
    ASSERT_EQ(actual, -1);
    ASSERT_EQ(r_err_fake.call_count, 1);
}

TEST_F(fs_posix_tests, file_store_valid_backup_and_write_succeed)
{
    // Arrange
    const int datalen = 4;
    char data[datalen+1] = "data";
    SetupFiles();
    std::string backupfilePath = file1Path + ".backup";

    // Act
    int actual = file_store(file1Path.c_str(), data, datalen, 0);

    std::string expected_string(data);
    std::string actual_string = ReadFileAsString(file1Path);

    std::string expected_backup_string(filePath1Data);
    std::string actual_backup_string = ReadFileAsString(backupfilePath);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(actual_string, expected_string);
    ASSERT_EQ(actual_backup_string, expected_backup_string);
    ASSERT_EQ(r_err_fake.call_count, 0);
}