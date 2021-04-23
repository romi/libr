#include <string>

#include "gtest/gtest.h"

#include "mock_clock.h"

#include "ClockAccessor.h"
#include "Clock.h"

using namespace testing;
#define NANOSECONDS_IN_SECOND  1000000000

class clock_tests : public ::testing::Test
{
protected:
    clock_tests() : mockClock(){
    }

    ~clock_tests() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
            // stop perceived memory leak since the global is static.
            rpp::ClockAccessor::SetInstance(nullptr);
    }
    std::shared_ptr<rpp::MockClock> mockClock;

};

// Old clock functions to test against.
uint64_t old_clock_timestamp()
{
        uint64_t t;
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        t = (uint64_t)ts.tv_sec * (uint64_t)NANOSECONDS_IN_SECOND + (uint64_t)ts.tv_nsec;
        return t;
}

double old_clock_time()
{
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return (double) ts.tv_sec + (double) ts.tv_nsec / NANOSECONDS_IN_SECOND;
}

char *old_clock_datetime_compact(char *buf, size_t len)
{

        struct tm r;
        struct timeval tv;
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        tv.tv_sec = ts.tv_sec;
        tv.tv_usec =ts.tv_nsec / 1000;

        localtime_r(&tv.tv_sec, &r);

        snprintf(buf, len, "%04d%02d%02d-%02d%02d%02d",
                 1900 + r.tm_year, 1 + r.tm_mon, r.tm_mday,
                 r.tm_hour, r.tm_min, r.tm_sec);
        return buf;
}
//////////////////////////////////////////////////// END OLD FUNCTIONS

TEST_F(clock_tests, clock_accessor_unset_returns_valid_clock_time_is_good)
{
        // Arrange;
        // Act
        auto clock = rpp::ClockAccessor::GetInstance();
        auto actual_string = clock->datetime_compact_string();

        char time[64] = {0};
        std::string oldstyle = old_clock_datetime_compact(time,64);
        // Not good tests but prove we have an instance.
        ASSERT_EQ(actual_string, oldstyle);
}

TEST_F(clock_tests, clock_accessor_returs_clock)
{
        // Arrange
        mockClock = std::make_shared<rpp::MockClock>();
        rpp::ClockAccessor::SetInstance(mockClock);

        double expected(0.123);
        std::string expected_string("expected");
        EXPECT_CALL(*mockClock, time)
                        .WillOnce(Return(expected));
        EXPECT_CALL(*mockClock, datetime_compact_string)
                        .WillOnce(Return(expected_string));

        // Act
        auto clock = rpp::ClockAccessor::GetInstance();
        auto actual = clock->time();
        auto actual_string = clock->datetime_compact_string();

        // Assert
        ASSERT_EQ(actual, expected);
        ASSERT_EQ(actual_string, expected_string);
}

TEST_F(clock_tests, clock_now_returns_time_matches_old_function)
{
        // Arrange
        rpp::Clock clock;

        // Act
        auto actual = clock.time();
        auto oldtime = old_clock_time();

        ASSERT_NEAR(actual, oldtime, 0.01);
}

TEST_F(clock_tests, clock_now_compactstring_returns_time_matches_old_function)
{
        // Arrange
        rpp::Clock clock;

        // Act
        auto actual = clock.datetime_compact_string();
        char time[64] = {0};
        std::string oldstyle = old_clock_datetime_compact(time,64);

        // Assert
        // Second granulatiry so they should match
        ASSERT_EQ(actual, oldstyle);
}

TEST_F(clock_tests, clock_timestamp_returns_timestamp)
{
        // Arrange
        rpp::Clock clock;

        // Act
        auto actual = clock.timestamp();
        auto old = old_clock_timestamp();

        // Assert
        // VERY PROCESSOR DEPENDENT. NOT A GOOD TEST. PLACEHOLDER.
        ASSERT_NEAR(static_cast<double>(actual), static_cast<double>(old), 100000);
}

TEST_F(clock_tests, clock_sleeps)
{
        // Arrange
        rpp::Clock clock;
        double sleeptime = 0.1;

        // Act
        auto start = clock.timestamp();
        clock.sleep(sleeptime);
        auto stop = clock.timestamp();
        auto duration = stop - start;

        auto expected = NANOSECONDS_IN_SECOND * sleeptime;

        // Assert
        // VERY PROCESSOR DEPENDENT. NOT A GOOD TEST. PLACEHOLDER.
        ASSERT_NEAR(static_cast<double>(duration), expected, NANOSECONDS_IN_SECOND/10);
}