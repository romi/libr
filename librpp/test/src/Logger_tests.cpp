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
        std::scoped_lock lock(Logger::log_mutex_);
        rpp::Logger::logger_ = std::shared_ptr<rpp::Logger>(new Logger(factory));
    }

    void clear_instance() {
        std::scoped_lock lock(Logger::log_mutex_);
        rpp::Logger::logger_ = nullptr;
    }

}

class Logger_tests : public ::testing::Test
{
protected:
    Logger_tests() : filename_("log.txt"), expected_DTC("DTC"),
        mockClock(), mockLogWriterFactory(), mockLogWriter(), log_buffer(){
    }

    ~Logger_tests() override = default;

    void SetUp() override
    {
        mockClock = std::make_shared<rpp::MockClock>();
        mockLogWriterFactory = std::make_shared<rpp::MockLogWriterFactory>();
        mockLogWriter = std::make_shared<rpp::MockLogWriter>();
        rpp::ClockAccessor::SetInstance(mockClock);
        set_log_capture();
        log_buffer = "";
    }

    void TearDown() override
    {
        // stop perceived memory leak since the global is static.
        rpp::ClockAccessor::SetInstance(nullptr);
        rpp::clear_instance();
        remove(filename_.c_str());
        log_cleanup();
    }

    void create_test_log_instance()
    {
        EXPECT_CALL(*mockLogWriterFactory, create_console_writer())
                .WillOnce(Return(mockLogWriter));
        rpp::set_instance(mockLogWriterFactory);
    };

    void set_default_expectations()
    {
        EXPECT_CALL(*mockClock, datetime_compact_string)
                .WillOnce(Return(expected_DTC));
    }

    void set_log_capture()
    {
        EXPECT_CALL(*mockLogWriter, write(_))
                .WillRepeatedly(Invoke([&log_buffer = log_buffer](const std::string& logged_message) {
                    log_buffer += logged_message;
                    return true;
                }));
    }

    const std::string filename_;
    const std::string expected_DTC;
    std::shared_ptr<rpp::MockClock> mockClock;
    std::shared_ptr<rpp::MockLogWriterFactory> mockLogWriterFactory;
    std::shared_ptr<rpp::MockLogWriter> mockLogWriter;
    std::string log_buffer;

};

// TBD: Needs more tests when logger is moved to C++ and mocked.
// Test exception throwing.

TEST_F(Logger_tests, constructor_creates_console_writer)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto expected = "LogMessage";

    // Act
    rpp::Logger::Instance()->log(rpp::log_level::DEBUG, expected);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}


TEST_F(Logger_tests, logger_outputs_compact_date_time)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    // Act
    rpp::Logger::Instance()->log(rpp::log_level::DEBUG, message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected_DTC));
}

TEST_F(Logger_tests, logger_outputs_correct_level_DEBUG)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "DD";
    // Act
    rpp::Logger::Instance()->log(rpp::log_level::DEBUG, message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}

TEST_F(Logger_tests, logger_outputs_correct_level_INFO)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "II";
    // Act
    rpp::Logger::Instance()->log(rpp::log_level::INFO, message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}

TEST_F(Logger_tests, logger_outputs_correct_level_WARNING)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "WW";
    // Act
    rpp::Logger::Instance()->log(rpp::log_level::WARNING, message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}

TEST_F(Logger_tests, logger_outputs_correct_level_ERROR)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "EE";
    // Act
    rpp::Logger::Instance()->log(rpp::log_level::ERROR, message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}

TEST_F(Logger_tests, logger_outputs_r_err)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "EE";
    // Act
    r_err("Test format %s", message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}

TEST_F(Logger_tests, logger_outputs_r_dbg)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "DD";
    // Act
    r_debug("Test format %s", message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}

TEST_F(Logger_tests, logger_outputs_r_info)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "II";
    // Act
    r_info("Test format %s", message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}

TEST_F(Logger_tests, logger_outputs_r_warn)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "WW";
    // Act
    r_warn("Test format %s", message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected));
}


TEST_F(Logger_tests, logger_sets_application_name)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    auto message = "LogMessage";
    std::string expected = "WW";
    std::string expected_name = "ApplicationName";
    // Act
    log_set_application(expected_name);
    r_info("Test format %s", message);

    // Assert
    ASSERT_THAT(log_buffer, HasSubstr(expected_name));
}

TEST_F(Logger_tests, logger_log_set_file_creates_file_writer)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    std::string expected = "WW";
    std::string log_filename = "log.txt";
    std::string_view  actual_filename;
    EXPECT_CALL(*mockLogWriter, close());
    EXPECT_CALL(*mockLogWriterFactory, create_file_writer())
            .WillOnce(Return(mockLogWriter));
    EXPECT_CALL(*mockLogWriter, open(_)).WillOnce(
            Invoke([&actual_filename](std::string_view filename)
            {actual_filename = filename;}));
    // Act
    log_set_file(log_filename);

    // Assert
    ASSERT_EQ(actual_filename, log_filename);
}

TEST_F(Logger_tests, logger_log_set_console_creates_console_writer)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    EXPECT_CALL(*mockLogWriter, close());
    EXPECT_CALL(*mockLogWriterFactory, create_console_writer())
            .WillOnce(Return(mockLogWriter));
    // Act
    // Assert
    log_set_console();
}

TEST_F(Logger_tests, move_log_catches_filesystem_exception)
{
    // Arrange
    set_default_expectations();
    create_test_log_instance();
    std::filesystem::filesystem_error filesystemError("Mock Error",
                                                      std::make_error_code(std::errc::no_such_file_or_directory));
    EXPECT_CALL(*mockLogWriter, close())
                .Times(2);

    EXPECT_CALL(*mockLogWriterFactory, create_file_writer())
            .WillOnce(Return(mockLogWriter));
    EXPECT_CALL(*mockLogWriter, open(_)).WillOnce(Throw(filesystemError));
    std::filesystem::path new_log_path("/tmpdoesnt_exist");

    // Act
    // Assert
    ASSERT_THROW(log_move(new_log_path),  std::filesystem::filesystem_error);
}


TEST_F(Logger_tests, logger_log_to_file_appends_log_to_same_file)
{
    // Arrange
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillRepeatedly(Return(expected_DTC));
    // Clear our fake instance so the next instance is created as a real class.
    rpp::clear_instance();

    // Act
    log_set_file("log.txt");
    r_info("Logging to file.");
    log_set_file("log.txt");
    r_info("Logging to file again.");
    // Assert
    ASSERT_TRUE(exists(std::filesystem::path("log.txt")));
}

TEST_F(Logger_tests, move_log_succeeds_when_logger_initialised_to_file)
{
    // Arrange
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillRepeatedly(Return(expected_DTC));
    // Clear our fake instance so the next instance is created as a real class.
    rpp::clear_instance();

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

    log_cleanup();
    remove(movedpath.c_str());
}

TEST_F(Logger_tests, move_log_suceeds_when_logger_initialised_to_console)
{
    // Arrange
    EXPECT_CALL(*mockClock, datetime_compact_string)
            .WillRepeatedly(Return(expected_DTC));
    // Clear our fake instance so the next instance is created as a real class.
    rpp::clear_instance();

    std::string original_log_path("./log.txt");
    std::string new_log_path("/tmp");
    std::string test_log1 ("Log1");
    std::string test_log2 ("Log2");
    std::string LogText{};

    log_cleanup();
    auto currentpath = log_get_file();
    r_info(test_log1);

    // Act
    log_move(new_log_path);
    r_info(test_log2);
    std::filesystem::path movedpath(log_get_file());

    //Assert
    ASSERT_TRUE(currentpath.empty());
    ASSERT_TRUE(std::filesystem::exists(movedpath));
    ASSERT_NO_THROW(LogText = FileUtils::TryReadFileAsString(movedpath));
    ASSERT_EQ(LogText.find(test_log1), std::string::npos);
    ASSERT_NE(LogText.find(test_log2), std::string::npos);
    log_cleanup();
    remove(movedpath.c_str());
}