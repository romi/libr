#include <string>
#include "gtest/gtest.h"

#include "clock.h"

extern "C" {
#include "log.mock.h"
#include "os_wrapper.mock.h"
}

class clock_posix_tests : public ::testing::Test
{
protected:
    clock_posix_tests() = default;

    ~clock_posix_tests() override = default;

    void SetUp() override
    {
        RESET_FAKE(gettimeofday_wrapper);
        RESET_FAKE(localtime_r_wrapper);
        RESET_FAKE(usleep_wrapper);

        fake_time_return_value = 0;
        fake_time.tv_sec = 0;
        fake_time.tv_usec = 0;

    }

    void TearDown() override
    {
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

};

struct timeval clock_posix_tests::fake_time;
int    clock_posix_tests::fake_time_return_value;


TEST_F(clock_posix_tests, clock_timestamp_is_correct)
{
    // Arrange
    fake_time.tv_sec = 1;
    fake_time.tv_usec = 1;
    gettimeofday_wrapper_fake.return_val = 0;
    gettimeofday_wrapper_fake.custom_fake = clock_posix_tests::gettimeofday_wrapper_custom_fake;

    uint64_t expected = (fake_time.tv_sec * MICROSECONDS_IN_SECOND) + fake_time.tv_usec;

    // Act
    uint64_t actual = clock_timestamp();

    //Assert
    ASSERT_EQ(gettimeofday_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_times_is_correct)
{
    // Arrange
    fake_time.tv_sec = 1;
    fake_time.tv_usec = 1;
    gettimeofday_wrapper_fake.return_val = 0;
    gettimeofday_wrapper_fake.custom_fake = clock_posix_tests::gettimeofday_wrapper_custom_fake;

    double expected = (double) fake_time.tv_sec + ((double) fake_time.tv_usec / MICROSECONDS_IN_SECOND);

    // Act
   double actual = clock_time();

    //Assert
    ASSERT_EQ(gettimeofday_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_datetime_format_is_correct)
{
    // Arrange
    char buffer[64];
    // 20 seconds into 1970.  01/01/1970 00:00:20
    fake_time.tv_sec = 20;
    fake_time.tv_usec = 1;
    gettimeofday_wrapper_fake.custom_fake = clock_posix_tests::gettimeofday_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;
    std::string expected("1970/01/01 00:00:20");

    // Act
    char* datetime = clock_datetime(buffer, sizeof(buffer), '/', ' ', ':');
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(gettimeofday_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_datetime_format_is_correct_when_short_buffer)
{
    // Arrange
    char buffer[64];
    fake_time.tv_sec = 20;
    fake_time.tv_usec = 1;
    gettimeofday_wrapper_fake.custom_fake = clock_posix_tests::gettimeofday_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;

    int buffsize = 8;
    std::string expected("1970/01");

    // Act
    char* datetime = clock_datetime(buffer, buffsize, '/', ' ', ':');
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(gettimeofday_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_log_datetime_format_is_correct)
{
    // Arrange
    char buffer[64];
    // 20 seconds into 1970.  01/01/1970 00:00:20
    fake_time.tv_sec = 20;
    fake_time.tv_usec = 30000;
    gettimeofday_wrapper_fake.custom_fake = clock_posix_tests::gettimeofday_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;
    std::string expected("1970/01/01 00:00:20:030");

    // Act
    char* datetime = clock_log_datetime(buffer, sizeof(buffer), '/', ' ', ':');
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(gettimeofday_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_log_datetime_format_is_correct_when_short_buffer)
{
    // Arrange
    char buffer[64];
    fake_time.tv_sec = 20;
    fake_time.tv_usec = 30000;
    gettimeofday_wrapper_fake.custom_fake = clock_posix_tests::gettimeofday_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;

    int buffsize = 8;
    std::string expected("1970/01");

    // Act
    char* datetime = clock_datetime(buffer, buffsize, '/', ' ', ':');
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(gettimeofday_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_sleep_calls_usleep_with_correct_time)
{
    // Arrange
    usleep_wrapper_fake.return_val = 0;
    double time_seconds = 2.1;
    __useconds_t expected = time_seconds * MICROSECONDS_IN_SECOND;

    // Act
    clock_sleep(time_seconds);

    //Assert
    ASSERT_EQ(usleep_wrapper_fake.call_count, 1);
    ASSERT_EQ(usleep_wrapper_fake.arg0_val, expected);
}