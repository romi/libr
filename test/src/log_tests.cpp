#include <experimental/filesystem>
#include <string>
#include "gtest/gtest.h"
#include "test_file_utils.h"

#include "log.h"

extern "C" {
#include "fff.h"
#include "os_wrapper.h"
#include <time.h>
#include <sys/time.h>
}

FAKE_VALUE_FUNC(int, gettimeofday_wrapper, struct timeval *, __timezone_ptr_t)
FAKE_VALUE_FUNC(struct tm * , localtime_r_wrapper, const time_t *, struct tm *)

namespace fs = std::experimental::filesystem;

class log_tests : public ::testing::Test
{
protected:
    log_tests() = default;

    ~log_tests() override = default;

    void SetUp() override
    {
        RESET_FAKE(gettimeofday_wrapper);
        RESET_FAKE(localtime_r_wrapper);

        if (!fs::is_directory(logDirectory.c_str()))
        {
            mkdir(logDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        // 20 seconds into 1970.  01/01/1970 00:00:20
        fake_time.tv_sec = 20;
        fake_time.tv_usec = 1;
        gettimeofday_wrapper_fake.custom_fake = log_tests::gettimeofday_wrapper_custom_fake;
        localtime_r_wrapper_fake.custom_fake = log_tests::localtime_r_wrapper_custom_fake;

        log_time = "1970-01-01 00:00:20";
        log_app_name = "?";
    }

    void TearDown() override
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

    static int gettimeofday_wrapper_custom_fake(struct timeval *__restrict tv, __timezone_ptr_t tz __attribute__((unused)) )
    {
        tv->tv_sec = fake_time.tv_sec;
        tv->tv_usec = fake_time.tv_usec;
        return fake_time_return_value;
    }

    static struct tm * localtime_r_wrapper_custom_fake(const time_t *timein, struct tm *timeout)
    {
        // Use gmtime for the conversion as localtime will apply DST to the time, which is not what we want for a mock.
        struct tm *gm = gmtime(timein);
        memcpy(timeout, gm, sizeof(struct tm));
        return timeout;
    }

    static struct timeval fake_time;
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
};

struct timeval log_tests::fake_time;
int    log_tests::fake_time_return_value;

TEST_F(log_tests, log_set_file_opens_log_file)
{
    // Arrange
    int init = r_log_init();
    std::string expected_log = CreateLogEntry(log_time, info_type, log_app_name, log_started);

    // Act
    auto actual = r_log_set_file(logfilepath.c_str());

    std::string actual_log;
    ASSERT_NO_THROW(actual_log = ReadFileAsString(logfilepath));

    //Assert
    ASSERT_EQ(init, 0);
    ASSERT_EQ(actual,0);
    ASSERT_EQ(actual_log, expected_log);

    r_log_cleanup();
}
