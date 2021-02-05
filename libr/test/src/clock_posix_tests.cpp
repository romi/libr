#include <string>
#include <time.h>
#include "gtest/gtest.h"

#include "clock_posix.h"

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
        RESET_FAKE(clock_gettime_wrapper);
        RESET_FAKE(localtime_r_wrapper);
        RESET_FAKE(usleep_wrapper);

        fake_time_return_value = 0;
        fake_time.tv_sec = 0;
        fake_time.tv_nsec = 0;

    }

    void TearDown() override
    {
    }

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
    static int             fake_time_return_value;

};

struct timespec clock_posix_tests::fake_time;
int    clock_posix_tests::fake_time_return_value;


TEST_F(clock_posix_tests, clock_timestamp_is_correct)
{
    // Arrange
    fake_time.tv_sec = 1;
    //fake_time.tv_nsec = 1;
    fake_time.tv_nsec = 1000;
    clock_gettime_wrapper_fake.return_val = 0;
    clock_gettime_wrapper_fake.custom_fake = clock_posix_tests::clock_gettime_wrapper_custom_fake;

    uint64_t expected = (uint64_t)((fake_time.tv_sec * NANOSECONDS_IN_SECOND) + fake_time.tv_nsec);

    // Act
    uint64_t actual = clock_timestamp();

    //Assert
    ASSERT_EQ(clock_gettime_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_times_is_correct)
{
    // Arrange
    fake_time.tv_sec = 1;
    fake_time.tv_nsec = 1000;
    clock_gettime_wrapper_fake.return_val = 0;
    clock_gettime_wrapper_fake.custom_fake = clock_posix_tests::clock_gettime_wrapper_custom_fake;

    double expected = (double) fake_time.tv_sec + ((double) fake_time.tv_nsec / NANOSECONDS_IN_SECOND);

    // Act
   double actual = clock_time();

    //Assert
    ASSERT_EQ(clock_gettime_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_datetime_format_is_correct)
{
    // Arrange
    char buffer[64];
    // 20 seconds into 1970.  01/01/1970 00:00:20
    fake_time.tv_sec = 20;
    fake_time.tv_nsec = 1000;
    clock_gettime_wrapper_fake.custom_fake = clock_posix_tests::clock_gettime_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;
    std::string expected("1970/01/01 00:00:20");

    // Act
    char* datetime = clock_datetime(buffer, sizeof(buffer), '/', ' ', ':');
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(clock_gettime_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_datetime_format_is_correct_when_short_buffer)
{
    // Arrange
    char buffer[64];
    fake_time.tv_sec = 20;
    fake_time.tv_nsec = 1000;
    clock_gettime_wrapper_fake.custom_fake = clock_posix_tests::clock_gettime_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;

    size_t buffsize = 8;
    std::string expected("1970/01");

    // Act
    char* datetime = clock_datetime(buffer, buffsize, '/', ' ', ':');
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(clock_gettime_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_log_datetime_format_is_correct)
{
    // Arrange
    char buffer[64];
    // 20 seconds into 1970.  01/01/1970 00:00:20
    fake_time.tv_sec = 20;
    fake_time.tv_nsec = 30000 * 1000;
    clock_gettime_wrapper_fake.custom_fake = clock_posix_tests::clock_gettime_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;
    std::string expected("1970/01/01 00:00:20.030");

    // Act
    char* datetime = clock_log_datetime(buffer, sizeof(buffer), '/', ' ', ':');
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(clock_gettime_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_log_datetime_format_is_correct_when_short_buffer)
{
    // Arrange
    char buffer[64];
    fake_time.tv_sec = 20;
    fake_time.tv_nsec = 30000 * 1000;
    clock_gettime_wrapper_fake.custom_fake = clock_posix_tests::clock_gettime_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;

    size_t buffsize = 8;
    std::string expected("1970/01");

    // Act
    char* datetime = clock_datetime(buffer, buffsize, '/', ' ', ':');
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(clock_gettime_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_datetime_compact_format_is_correct)
{
    // Arrange
    char buffer[64];
    // 20 seconds into 1970.  01/01/1970 00:00:20
    fake_time.tv_sec = 20;
    fake_time.tv_nsec = 1000;
    clock_gettime_wrapper_fake.custom_fake = clock_posix_tests::clock_gettime_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;
    std::string expected("19700101-000020");

    // Act
    char* datetime = clock_datetime_compact(buffer, sizeof(buffer));
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(clock_gettime_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}

TEST_F(clock_posix_tests, clock_datetime_compact_format_is_correct_when_short_buffer)
{
    // Arrange
    char buffer[64];
    fake_time.tv_sec = 20;
    fake_time.tv_nsec = 1000;
    clock_gettime_wrapper_fake.custom_fake = clock_posix_tests::clock_gettime_wrapper_custom_fake;
    localtime_r_wrapper_fake.custom_fake = clock_posix_tests::localtime_r_wrapper_custom_fake;

    size_t buffsize = 7;
    std::string expected("197001");

    // Act
    char* datetime = clock_datetime_compact(buffer, buffsize);
    std::string actual(datetime);

    //Assert
    ASSERT_EQ(clock_gettime_wrapper_fake.call_count, 1);
    ASSERT_EQ(localtime_r_wrapper_fake.call_count, 1);
    ASSERT_EQ(actual, expected);
}


TEST_F(clock_posix_tests, clock_sleep_calls_usleep_with_correct_time)
{
    // Arrange
    usleep_wrapper_fake.return_val = 0;
    double time_seconds = 2.1;
    __useconds_t expected = __useconds_t(time_seconds * MICROSECONDS_IN_SECOND);

    // Act
    clock_sleep(time_seconds);

    //Assert
    ASSERT_EQ(usleep_wrapper_fake.call_count, 1);
    ASSERT_EQ(usleep_wrapper_fake.arg0_val, expected);
}
