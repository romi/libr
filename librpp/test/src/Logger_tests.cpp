#include <string>
#include "Logger.h"
#include "FileUtils.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ClockAccessor.h"
#include "mock_clock.h"
#include "mock_logwriter.h"
#include "mock_logwriterfactory.h"

using namespace testing;
using ::testing::HasSubstr;

class Logger_tests : public ::testing::Test
{
protected:
    Logger_tests() : mockClock(), mockLogWriterFactory(), mockLogWriter(){
    }

    ~Logger_tests() override = default;

    void SetUp() override
    {
        mockClock = std::make_shared<rpp::MockClock>();
        rpp::ClockAccessor::SetInstance(mockClock);
    }

    void TearDown() override
    {
        // stop perceived memory leak since the global is static.
        rpp::ClockAccessor::SetInstance(nullptr);
    }

    std::shared_ptr<rpp::MockClock> mockClock;
    std::shared_ptr<rpp::MockLogWriterFactory> mockLogWriterFactory;
    std::shared_ptr<rpp::MockLogWriter> mockLogWriter;
};

// TBD: Needs more tests when logger is mpved to C++ and mocked.
// Test exception throwing.
TEST_F(Logger_tests, constructor_sets_output_to_console)
{
    // Arrange
    auto logger = rpp::Logger::Instance();
    auto message = "Console";
    testing::internal::CaptureStdout();
    // Act
    logger->log(rpp::log_level::DEBUG, message);

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(message));
}

TEST_F(Logger_tests, logger_outputs_compact_date_time)
{
    // Arrange
    auto logger = rpp::Logger::Instance();
    std::string expected_string = "DTC";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return(expected_string));
    auto message = "Console";
    testing::internal::CaptureStdout();
    // Act
    logger->log(rpp::log_level::DEBUG, message);

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, logger_outputs_correct_level_DEBUG)
{
    // Arrange
    auto logger = rpp::Logger::Instance();
    std::string expected_string = "DD";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return(expected_string));
    auto message = "Console";
    testing::internal::CaptureStdout();
    // Act
    logger->log(rpp::log_level::DEBUG, message);

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, logger_outputs_correct_level_INFO)
{
    // Arrange
    auto logger = rpp::Logger::Instance();
    std::string expected_string = "II";
    auto message = "Console";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return("DTS"));
    testing::internal::CaptureStdout();
    // Act
    logger->log(rpp::log_level::INFO, message);

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, logger_outputs_correct_level_WARNING)
{
    // Arrange
    auto logger = rpp::Logger::Instance();
    std::string expected_string = "WW";
    auto message = "Console";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return("DTS"));
    testing::internal::CaptureStdout();
    // Act
    logger->log(rpp::log_level::WARNING, message);

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, logger_outputs_correct_level_ERROR)
{
    // Arrange
    auto logger = rpp::Logger::Instance();
    std::string expected_string = "EE";
    auto message = "Console";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return("DTS"));
    testing::internal::CaptureStdout();
    // Act
    logger->log(rpp::log_level::ERROR, message);

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, logger_sets_application_name)
{
    // Arrange
    auto logger = rpp::Logger::Instance();
    std::string expected_string = "ApplicationName";
    auto message = "Console";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return("DTS"));
    testing::internal::CaptureStdout();
    // Act
    log_set_application(expected_string);
    logger->log(rpp::log_level::ERROR, message);

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, move_log_suceeds_when_logger_initialised)
{
    // Arrange
    std::string original_log_path("./log.txt");
    std::filesystem::path new_log_path("/tmp");
    std::string test_string ("TESTSTRING");
    std::string LogText{};

    log_init();
    log_set_file(original_log_path.c_str());
    std::filesystem::path currentpath(log_get_file());

    // Act
    rpp::Logger::MoveLog(new_log_path);
    r_info(test_string.c_str());
    std::filesystem::path movedpath(log_get_file());

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

    log_cleanup();
    auto currentpath = log_get_file();

    // Act
    rpp::Logger::MoveLog(new_log_path);
    r_info(test_string.c_str());
    std::filesystem::path movedpath(log_get_file());

    //Assert
    ASSERT_TRUE(currentpath.empty());
    ASSERT_TRUE(std::filesystem::exists(movedpath));
    ASSERT_NO_THROW(LogText = FileUtils::TryReadFileAsString(movedpath));
    ASSERT_NE(LogText.find(test_string), std::string::npos);
}