#include <experimental/filesystem>
#include <string>
#include "gtest/gtest.h"
#include "test_file_utils.h"

#include "log.h"

extern "C" {
#include "fff.h"
#include "os_wrapper.h"
#include <ctime>
#include <sys/time.h>
}

FAKE_VALUE_FUNC(int, clock_gettime_wrapper, clockid_t, struct timespec *)
FAKE_VALUE_FUNC(struct tm * , localtime_r_wrapper, const time_t *, struct tm *)

namespace fs = std::experimental::filesystem;

class log_tests : public ::testing::Test
{
protected:
    log_tests() = default;

    ~log_tests() override = default;

    void SetUp() override
    {
        RESET_FAKE(clock_gettime_wrapper);
        RESET_FAKE(localtime_r_wrapper);

        RemoveLogDirectory();

        if (!fs::is_directory(logDirectory.c_str()))
        {
            mkdir(logDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        // 20 seconds into 1970.  01/01/1970 00:00:20
        fake_time.tv_sec = 20;
        fake_time.tv_nsec = 30000 * 1000;
        clock_gettime_wrapper_fake.custom_fake = log_tests::clock_gettime_wrapper_custom_fake;
        localtime_r_wrapper_fake.custom_fake = log_tests::localtime_r_wrapper_custom_fake;

        log_time = "1970-01-01 00:00:20.030";
        log_app_name = "?";

        CreateChangingLogFile(logfilepath);
    }

    void TearDown() override
    {
        RemoveLogDirectory();
    }

    void RemoveLogDirectory()
    {
        if (fs::is_directory(fs::path(logDirectory)))
        {
            fs::remove_all(logDirectory);
        }
    }

    std::string CreateLogEntry(const std::string& time, const std::string& type, const std::string& name, const std::string& log)
    {
        auto size = std::snprintf(nullptr, 0, logformat.c_str(), time.c_str(), type.c_str(), name.c_str(), log.c_str());
        std::string logEntry(size, '\0');
        std::sprintf(&logEntry[0], logformat.c_str(), time.c_str(), type.c_str(), name.c_str(), log.c_str());
        return logEntry;
    }

    std::string CreateChangingLogFile(const std::string &path) {
        changing_log_file_entry = "Changing log to '";
        changing_log_file_entry += path;
        changing_log_file_entry += "'";
        return changing_log_file_entry;
    }

//    static int gettimeofday_wrapper_custom_fake(struct timeval *__restrict tv, __timezone_ptr_t tz __attribute__((unused)) )
//    {
//        tv->tv_sec = fake_time.tv_sec;
//        tv->tv_usec = fake_time.tv_usec;
//        return fake_time_return_value;
//    }

    static int clock_gettime_wrapper_custom_fake(clockid_t clockId __attribute__((unused)), struct timespec *ts )
    {

        ts->tv_sec = fake_time.tv_sec;
        ts->tv_nsec = fake_time.tv_nsec;
        return fake_time_return_value;
    }

    static struct tm * localtime_r_wrapper_custom_fake(const time_t *timein, struct tm *timeout)
    {
        // Use gmtime for the conversion as localtime will apply DST to the time, which is not what we want for a mock.
        struct tm *gm = gmtime(timein);
        memcpy(timeout, gm, sizeof(struct tm));
        return timeout;
    }

    static struct timespec fake_time;
    static int            fake_time_return_value;

    const std::string logDirectory = "./logDirectory/";
    const std::string logfilepath = logDirectory + std::string("logfile");
    const std::string logformat = "[%s] [%s] [%s] %s\n";
    const std::string debug_type = "DD";
    const std::string info_type = "II";
    const std::string warning_type = "WW";
    const std::string error_type= "EE";
    const std::string panic_type = "!!";
    const std::string unknown_type = "Unknown";
    const std::string log_started = "Log started ----------------------------------------";
    std::string log_time;
    std::string log_app_name;
    std::string changing_log_file_entry;
    std::string log_string_from_callback;

public:
    void set_last_log_entry_from_callback(const char *logstring)
    {
        log_string_from_callback = std::string(logstring) + '\n';
    }
};

struct timespec log_tests::fake_time;
int    log_tests::fake_time_return_value;

TEST_F(log_tests, log_set_file_opens_log_file_writes_to_std_out)
{
    // Arrange
    int init = r_log_init();
    std::string expected_log = CreateLogEntry(log_time, info_type, log_app_name, log_started);
    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_file_entry);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    std::string actual_log;
    ASSERT_NO_THROW(actual_log = ReadFileAsString(logfilepath));

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_log, expected_log);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_get_file_gets_file)
{
    // Arrange
    int init = r_log_init();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    std::string filepath = r_log_get_file();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(filepath, logfilepath);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_file_invalid_path_fails_writes_to_std_err)
{
    // Arrange
    int init = r_log_init();
    testing::internal::CaptureStderr();

    std::string badpathname = "/somerandomdir/testfolder/logfile";
    std::string expected_stderr = "Failed to open logfile at '";
    expected_stderr += badpathname;
    expected_stderr += "'\n";

    // Act
    auto actual = r_log_set_file(badpathname.c_str());
    std::string actual_stderror = testing::internal::GetCapturedStderr();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,-1);
    ASSERT_EQ(actual_stderror, expected_stderr);

    r_log_cleanup();
}


TEST_F(log_tests, log_set_file_with_hyphen_does_not_open_file_opens_stdout)
{
    // Arrange
    int init = r_log_init();
    std::string logfilepath = "-";
    std::string changing_log_entry = CreateChangingLogFile(logfilepath);

    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_entry);
    expected_stdoutput += CreateLogEntry(log_time, info_type, log_app_name, log_started);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_file_again_opens_log_file_again)
{
    // Arrange
    const std::string secondlogfilepath = logDirectory + std::string("logfile2");

    std::string changing_log_entry = CreateChangingLogFile(secondlogfilepath);

    int init = r_log_init();
    std::string expected_log = CreateLogEntry(log_time, info_type, log_app_name, log_started);
    expected_log += CreateLogEntry(log_time, info_type, log_app_name, changing_log_file_entry);
    std::string expected_secondlog = CreateLogEntry(log_time, info_type, log_app_name, log_started);

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    auto actual2 = r_log_set_file(secondlogfilepath.c_str());

    std::string actual_log;
    ASSERT_NO_THROW(actual_log = ReadFileAsString(logfilepath));
    std::string actual_secondlog;
    ASSERT_NO_THROW(actual_secondlog = ReadFileAsString(secondlogfilepath));

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_log, expected_log);
    ASSERT_EQ(actual2,0);
    ASSERT_EQ(actual_secondlog, expected_secondlog);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_app_sets_app_field)
{
    // Arrange
    log_app_name = "test_app";
    int init = r_log_init();
    std::string expected_log = CreateLogEntry(log_time, info_type, log_app_name, log_started);

    // Act
    r_log_set_app(log_app_name.c_str());
    auto actual = r_log_set_file(logfilepath.c_str());

    std::string actual_log;
    ASSERT_NO_THROW(actual_log = ReadFileAsString(logfilepath));

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_log, expected_log);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_app_sets_app_field_again)
{
    // Arrange
    std::string expected_app_name = "new_test_app";
    log_app_name = "test_app";
    int init = r_log_init();
    std::string expected_log = CreateLogEntry(log_time, info_type, expected_app_name, log_started);

    // Act
    r_log_set_app(log_app_name.c_str());
    r_log_set_app(expected_app_name.c_str());
    auto actual = r_log_set_file(logfilepath.c_str());

    std::string actual_log;
    ASSERT_NO_THROW(actual_log = ReadFileAsString(logfilepath));

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_log, expected_log);

    r_log_cleanup();
}

TEST_F(log_tests, log_get_level_gets_level)
{
    // Arrange
    int init = r_log_init();

    // Act
    int actual = r_log_get_level();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,R_DEBUG);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_level_sets_level)
{
    // Arrange
    int level = R_INFO;
    int init = r_log_init();

    // Act
    r_log_set_level(level);
    int actual = r_log_get_level();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,level);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_level_panic_sets_level_error)
{
    // Arrange
    int level = R_ERROR;
    int init = r_log_init();

    // Act
    r_log_set_level(R_PANIC);
    int actual = r_log_get_level();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,level);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_level_too_low_sets_debug)
{
    // Arrange
    int level = -1;
    int init = r_log_init();

    // Act
    r_log_set_level(level);
    int actual = r_log_get_level();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,R_DEBUG);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_level_too_high_sets_error)
{
    // Arrange
    int level = 100;
    int init = r_log_init();

    // Act
    r_log_set_level(level);
    int actual = r_log_get_level();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,R_ERROR);

    r_log_cleanup();
}


void log_writer_callback(void *userdata, const char* s)
{
    auto* ptests = reinterpret_cast<log_tests*>(userdata);
    ptests->set_last_log_entry_from_callback(s);
}

TEST_F(log_tests, log_get_writer_gets_writer)
{
    // Arrange
    int init = r_log_init();

    log_writer_t callback = log_writer_callback;
    char buffer[] = "test";
    char *pbuffer = &buffer[0];
    char **ppbuffer = &pbuffer;

    // Act
    r_log_get_writer(&callback, reinterpret_cast<void **>(ppbuffer));

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(pbuffer, nullptr);
    ASSERT_EQ(callback, nullptr);

    r_log_cleanup();
}


TEST_F(log_tests, log_set_writer_sets_writer)
{
    // Arrange
    int init = r_log_init();

    log_writer_t callback = log_writer_callback;
    char buffer[] = "test";
    char *pbuffer = &buffer[0];

    log_writer_t callback_return = nullptr;
    char *pbuffer_return = nullptr;
    char **ppbuffer_return = &pbuffer_return;

    // Act
    r_log_set_writer(callback, (void *)pbuffer);
    r_log_get_writer(&callback_return, reinterpret_cast<void **>(ppbuffer_return));

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(*ppbuffer_return, pbuffer);
    ASSERT_EQ(callback_return, callback);

    r_log_cleanup();
}

TEST_F(log_tests, log_set_writer_when_writer_set_logs_to_writer)
{
    // Arrange
    int init = r_log_init();
    std::string log_entry = "last log entry";
    std::string expected_log = CreateLogEntry(log_time, info_type, log_app_name, log_entry);

    log_writer_t callback = log_writer_callback;

    // Act
    r_log_set_writer(callback, (void *)this);
    r_info("%s", log_entry.c_str());

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(expected_log, log_string_from_callback);

    r_log_cleanup();
}

TEST_F(log_tests, log_panic_when_file_set_logs_to_stdout_and_file)
{
    // Arrange
    std::string panic_string("PANIC!!");
    int init = r_log_init();
    std::string expected_log = CreateLogEntry(log_time, info_type, log_app_name, log_started);
    expected_log += CreateLogEntry(log_time, panic_type, log_app_name, panic_string);

    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_file_entry);
    expected_stdoutput += CreateLogEntry(log_time, panic_type, log_app_name, panic_string);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    r_panic("%s", panic_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    std::string actual_log;
    ASSERT_NO_THROW(actual_log = ReadFileAsString(logfilepath));

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_log, expected_log);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_panic_no_init_when_file_set_logs_to_stdout_and_file)
{
    // Arrange
    std::string panic_string("PANIC!!");
    std::string expected_log = CreateLogEntry(log_time, info_type, log_app_name, log_started);
    expected_log += CreateLogEntry(log_time, panic_type, log_app_name, panic_string);

    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_file_entry);
    expected_stdoutput += CreateLogEntry(log_time, panic_type, log_app_name, panic_string);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    r_panic("%s", panic_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    std::string actual_log;
    ASSERT_NO_THROW(actual_log = ReadFileAsString(logfilepath));

    //Assert
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_log, expected_log);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_panic_no_init_logs_to_stdout)
{
    // Arrange
    std::string panic_string("PANIC!!");
    std::string expected_stdoutput = CreateLogEntry(log_time, panic_type, log_app_name, panic_string);
    testing::internal::CaptureStdout();

    // Act
    r_panic("%s", panic_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_panic_when_no_file_set_logs_to_stdout_once_only)
{
    // Arrange
    std::string panic_string("PANIC!!");
    int init = r_log_init();
    std::string logfilepath = "-";
    std::string changing_log_entry = CreateChangingLogFile(logfilepath);

    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_entry);
    expected_stdoutput += CreateLogEntry(log_time, info_type, log_app_name, log_started);
    expected_stdoutput += CreateLogEntry(log_time, panic_type, log_app_name, panic_string);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    r_panic("%s", panic_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_error_logs_error)
{
    // Arrange
    std::string error_string("ERROR!!");
    int init = r_log_init();
    std::string logfilepath = "-";
    std::string changing_log_entry = CreateChangingLogFile(logfilepath);

    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_entry);
    expected_stdoutput += CreateLogEntry(log_time, info_type, log_app_name, log_started);
    expected_stdoutput += CreateLogEntry(log_time, error_type, log_app_name, error_string);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    r_err("%s", error_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_error_no_init_logs_to_stdout)
{
    // Arrange
    std::string error_string("PANIC!!");
    std::string expected_stdoutput = CreateLogEntry(log_time, error_type, log_app_name, error_string);
    testing::internal::CaptureStdout();

    // Act
    r_err("%s", error_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_warning_logs_warning)
{
    // Arrange
    std::string warning_string("WARNING!!");
    int init = r_log_init();
    std::string logfilepath = "-";
    std::string changing_log_entry = CreateChangingLogFile(logfilepath);

    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_entry);
    expected_stdoutput += CreateLogEntry(log_time, info_type, log_app_name, log_started);
    expected_stdoutput += CreateLogEntry(log_time, warning_type, log_app_name, warning_string);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    r_warn("%s", warning_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_warning_no_init_logs_to_stdout)
{
    // Arrange
    std::string warning_string("PANIC!!");
    std::string expected_stdoutput = CreateLogEntry(log_time, warning_type, log_app_name, warning_string);
    testing::internal::CaptureStdout();

    // Act
    r_warn("%s", warning_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_warning_level_error_no_logs_to_stdout)
{
    // Arrange
    std::string warning_string("PANIC!!");
    std::string expected_stdoutput;
    testing::internal::CaptureStdout();

    // Act
    r_log_set_level(R_ERROR);
    r_warn("%s", warning_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_info_logs_info)
{
    // Arrange
    std::string info_string("INFO!!");
    int init = r_log_init();
    std::string logfilepath = "-";
    std::string changing_log_entry = CreateChangingLogFile(logfilepath);

    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_entry);
    expected_stdoutput += CreateLogEntry(log_time, info_type, log_app_name, log_started);
    expected_stdoutput += CreateLogEntry(log_time, info_type, log_app_name, info_string);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    r_info("%s", info_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_info_no_init_logs_to_stdout)
{
    // Arrange
    std::string info_string("PANIC!!");
    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, info_string);
    testing::internal::CaptureStdout();

    // Act
    r_info("%s", info_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_info_level_error_no_logs_to_stdout)
{
    // Arrange
    std::string info_string("PANIC!!");
    std::string expected_stdoutput;
    testing::internal::CaptureStdout();

    // Act
    r_log_set_level(R_ERROR);
    r_info("%s", info_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_debug_logs_debug)
{
    // Arrange
    std::string debug_string("INFO!!");
    int init = r_log_init();
    std::string logfilepath = "-";
    std::string changing_log_entry = CreateChangingLogFile(logfilepath);

    std::string expected_stdoutput = CreateLogEntry(log_time, info_type, log_app_name, changing_log_entry);
    expected_stdoutput += CreateLogEntry(log_time, info_type, log_app_name, log_started);
    expected_stdoutput += CreateLogEntry(log_time, debug_type, log_app_name, debug_string);

    testing::internal::CaptureStdout();

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());
    r_debug("%s", debug_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);

    r_log_cleanup();
}

TEST_F(log_tests, log_debug_no_init_logs_to_stdout)
{
    // Arrange
    std::string debug_string("PANIC!!");
    std::string expected_stdoutput = CreateLogEntry(log_time, debug_type, log_app_name, debug_string);
    testing::internal::CaptureStdout();

    // Act
    r_debug("%s", debug_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_debug_level_error_no_logs_to_stdout)
{
    // Arrange
    std::string debug_string("PANIC!!");
    std::string expected_stdoutput;
    testing::internal::CaptureStdout();

    // Act
    r_log_set_level(R_ERROR);
    r_debug("%s", debug_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_1k_buffer_logs_1k_buffer_plus_log_stamp)
{
    // Arrange
    const int buffsize = 1024;
    char buffer[buffsize];
    memset(buffer, 'a', buffsize);
    buffer[buffsize-1] = 0;

    std::string debug_string(buffer);
    std::string expected_stdoutput = CreateLogEntry(log_time, panic_type, log_app_name, debug_string);

    testing::internal::CaptureStdout();

    // Act
    r_panic("%s", debug_string.c_str());
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}

TEST_F(log_tests, log_greater_than_1k_buffer_logs_1k_buffer_plus_log_stamp)
{
    // Arrange
    const int buffsize = 1024;
    char buffer[buffsize];
    memset(buffer, 'a', buffsize);
    buffer[buffsize-1] = 0;

    const int bigbuffsize = 1048;
    char bigbuffer[bigbuffsize];
    memset(bigbuffer, 'a', bigbuffsize);
    bigbuffer[bigbuffsize-1] = 0;

    std::string debug_string(buffer);
    std::string expected_stdoutput = CreateLogEntry(log_time, panic_type, log_app_name, debug_string);

    testing::internal::CaptureStdout();

    // Act
    r_panic("%s", bigbuffer);
    std::string actual_stdoutput = testing::internal::GetCapturedStdout();

    // Assert
    ASSERT_EQ(actual_stdoutput, expected_stdoutput);
    r_log_cleanup();
}
