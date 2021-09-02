#include <string>
#include "Logger.h"
#include "FileUtils.h"

#include "gtest/gtest.h"

using namespace testing;

class Logger_tests : public ::testing::Test
{
protected:
    Logger_tests() = default;

    ~Logger_tests() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {

    }
};

// TBD: Needs more tests when logger is mpved to C++ and mocked.
// Test exception throwing.
TEST_F(Logger_tests, move_log_suceeds_when_logger_initialised)
{
    // Arrange
    std::string original_log_path("./log.txt");
    std::string new_log_path("/tmp");
    std::string test_string ("TESTSTRING");
    std::string LogText{};

    r_log_init();
    r_log_set_file(original_log_path.c_str());
    std::filesystem::path currentpath(r_log_get_file());

    // Act
    rpp::Logger::MoveLog(new_log_path);
    r_info(test_string.c_str());
    std::filesystem::path movedpath(r_log_get_file());

    //Assert
    ASSERT_EQ(currentpath,original_log_path);
    ASSERT_TRUE(std::filesystem::exists(movedpath));
    ASSERT_FALSE(std::filesystem::exists(currentpath));
    ASSERT_NO_THROW(LogText = FileUtils::TryReadFileAsString(movedpath));
    ASSERT_NE(LogText.find(test_string), std::string::npos);
}


TEST_F(Logger_tests, move_log_suceeds_when_logger_not_initialised)
{
    // Arrange
    std::string original_log_path("./log.txt");
    std::string new_log_path("/tmp");
    std::string test_string ("TESTSTRING");
    std::string LogText{};

    r_log_cleanup();
    const char *currentpath = r_log_get_file();

    // Act
    rpp::Logger::MoveLog(new_log_path);
    r_info(test_string.c_str());
    std::filesystem::path movedpath(r_log_get_file());

    //Assert
    ASSERT_EQ(currentpath, nullptr);
    ASSERT_TRUE(std::filesystem::exists(movedpath));
    ASSERT_NO_THROW(LogText = FileUtils::TryReadFileAsString(movedpath));
    ASSERT_NE(LogText.find(test_string), std::string::npos);
}