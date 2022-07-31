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
using testing::Invoke;

namespace rpp {
    void set_instance(const std::shared_ptr<rpp::ILogWriterFactory>& factory) {
        rpp::Logger::logger_ = std::shared_ptr<rpp::Logger>(new Logger(factory));
    }

    void clear_instance() {
        rpp::Logger::logger_ = nullptr;
    }

}

class Logger_tests : public ::testing::Test
{
protected:
    Logger_tests() : filename_("log.txt"), mockClock(), mockLogWriterFactory(), mockLogWriter(){
    }

    ~Logger_tests() override = default;

    void SetUp() override
    {
        mockClock = std::make_shared<rpp::MockClock>();
        mockLogWriterFactory = std::make_shared<rpp::MockLogWriterFactory>();
        mockLogWriter = std::make_shared<rpp::MockLogWriter>();
        rpp::ClockAccessor::SetInstance(mockClock);
    }

    void TearDown() override
    {
        // stop perceived memory leak since the global is static.
        rpp::ClockAccessor::SetInstance(nullptr);
        remove(filename_.c_str());
    }


    std::string filename_;
    std::shared_ptr<rpp::MockClock> mockClock;
    std::shared_ptr<rpp::MockLogWriterFactory> mockLogWriterFactory;
    std::shared_ptr<rpp::MockLogWriter> mockLogWriter;

};

// TBD: Needs more tests when logger is moved to C++ and mocked.
// Test exception throwing.

TEST_F(Logger_tests, constructor_creates_log_writer_factory)
{
    // Arrange
    auto expected = "LogMessage";
    std::string actual;
    EXPECT_CALL(*mockLogWriterFactory, create_console_writer())
            .WillOnce(Return(mockLogWriter));
    std::string expected_string = "DTC";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return(expected_string));
    EXPECT_CALL(*mockLogWriter, write(_))
            .WillRepeatedly(Invoke([&actual](const std::string& logged_message) {
                actual += logged_message;
                return true;
            }));
    rpp::set_instance(mockLogWriterFactory);

    auto logger = rpp::Logger::Instance();
    // Act
    logger->log(rpp::log_level::DEBUG, expected);

    // Assert
    ASSERT_THAT(actual, HasSubstr(expected));
    rpp::clear_instance();
}

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

TEST_F(Logger_tests, logger_outputs_r_err)
{
    // Arrange
    std::string expected_string = "EE";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return("DTS"));
    testing::internal::CaptureStdout();
    // Act
    r_err("Test format %s", "string");

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, logger_outputs_r_dbg)
{
    // Arrange
    std::string expected_string = "DD";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return("DTS"));
    testing::internal::CaptureStdout();
    // Act
    r_debug("Test format %s", "string");

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, logger_outputs_r_info)
{
    // Arrange
    std::string expected_string = "II";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return("DTS"));
    testing::internal::CaptureStdout();
    // Act
    r_info("Test format %s", "string");

    // Assert
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();
    ASSERT_THAT(actual_stdoutput, HasSubstr(expected_string));
}

TEST_F(Logger_tests, logger_outputs_r_warn)
{
    // Arrange
    std::string expected_string = "WW";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillOnce(Return("DTS"));
    testing::internal::CaptureStdout();
    // Act
    r_warn("Test format %s", "string");

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

TEST_F(Logger_tests, logger_log_to_file_logs)
{
    // Arrange
    std::string expected_string = "ApplicationName";
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillRepeatedly(Return("DTS"));
    // Act
    log_set_application(expected_string);
    log_set_file("log.txt");
    r_info("Logging to file.");
    log_set_file("log.txt");
    r_info("Logging to file again.");
    // Assert
    ASSERT_TRUE(exists(std::filesystem::path("log.txt")));
}

TEST_F(Logger_tests, move_log_succeeds_when_logger_initialised)
{
    // Arrange
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillRepeatedly(Return("DTS"));
    std::string original_log_path("./log.txt");
    std::filesystem::path new_log_path("/tmp");
    std::string test_log1 ("Log1");
    std::string test_log2 ("Log2");
    std::string LogText{};

    log_init();
    log_set_file(original_log_path);
    r_info(test_log1);
    std::filesystem::path currentpath(log_get_file());

    // Act
    log_move(new_log_path);
    r_info(test_log2);
    std::filesystem::path movedpath(log_get_file());

    //Assert
    ASSERT_EQ(currentpath,original_log_path);
    ASSERT_TRUE(std::filesystem::exists(movedpath));
    ASSERT_FALSE(std::filesystem::exists(currentpath));
    ASSERT_NO_THROW(LogText = FileUtils::TryReadFileAsString(movedpath));
    ASSERT_NE(LogText.find(test_log1), std::string::npos);
    ASSERT_NE(LogText.find(test_log2), std::string::npos);
}


TEST_F(Logger_tests, move_log_suceeds_when_logger_not_initialised)
{
    // Arrange
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillRepeatedly(Return("DTS"));
    std::string original_log_path("./log.txt");
    std::string new_log_path("/tmp");
    std::string test_string ("TESTSTRING");
    std::string LogText{};

    log_cleanup();
    auto currentpath = log_get_file();

    // Act
    log_move(new_log_path);
    r_info(test_string);
    std::filesystem::path movedpath(log_get_file());

    //Assert
    ASSERT_TRUE(currentpath.empty());
    ASSERT_TRUE(std::filesystem::exists(movedpath));
    ASSERT_NO_THROW(LogText = FileUtils::TryReadFileAsString(movedpath));
    ASSERT_NE(LogText.find(test_string), std::string::npos);
}