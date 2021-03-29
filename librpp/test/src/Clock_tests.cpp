#include <string>

#include "gtest/gtest.h"

#include "fff.h"
DEFINE_FFF_GLOBALS

#include "mock_clock.h"
#include "clock_posix.mock.h"

#include "ClockAccessor.h"
#include "Clock.h"

using namespace testing;

class clock_tests : public ::testing::Test
{
protected:
    clock_tests() : mockClock(){
    }

    ~clock_tests() override = default;

    void SetUp() override
    {
            RESET_FAKE(clock_timestamp);
            RESET_FAKE(clock_time);
            RESET_FAKE(clock_datetime_compact);
            mockClock = std::make_shared<rpp::MockClock>();
            rpp::ClockAccessor::SetInstance(mockClock);
    }

    void TearDown() override
    {
            // Stop the memory leak since the global is static.
            rpp::ClockAccessor::SetInstance(nullptr);
    }
    std::shared_ptr<rpp::MockClock> mockClock;

};

TEST_F(clock_tests, clock_accessor_returs_clock)
{
        // Arrange
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

TEST_F(clock_tests, clock_now_returns_time)
{
        // Arrange
        double expected = 0.123;
        rpp::Clock clock;
        clock_time_fake.return_val = expected;

        // Act
        auto actual = clock.time();

        // Assert
        ASSERT_EQ(actual, expected);
}

TEST_F(clock_tests, clock_nowcpmpactstring_returns_time)
{
        // Arrange
        std::string expected("expected_time");
        rpp::Clock clock;
        clock_datetime_compact_fake.return_val = const_cast<char*>(expected.c_str());

        // Act
        auto actual = clock.datetime_compact_string();

        // Assert
        ASSERT_EQ(actual, expected);
}