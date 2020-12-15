#include <string>

#include "gtest/gtest.h"

#include "os_wrapper.h"
#include "clock.h"

extern "C" {
#include "log.mock.h"
}

/* Since all these wrapper functions are mocked they receive 0 coverage.
 * We are unable to exclude individual files so it's easier just to call the wrapper
 * functions to maintain coverage. This file performs no tests. */

class wrapper_tests_coverage : public ::testing::Test {
protected:
    wrapper_tests_coverage() = default;

    ~wrapper_tests_coverage() override = default;

    void SetUp() override
    {
        RESET_FAKE(r_err);
        RESET_FAKE(r_warn);
    }

    void TearDown() override
    {
    }
};

TEST_F(wrapper_tests_coverage, memory_wrapper_coverage)
{
    // Arrange
    size_t size = 1024;

    // Act
    void *data = malloc_wrapper(size);
    memset_wrapper(data, 0, size);
    data = realloc_wrapper(data, size*2);
    free_wrapper(data);

    // Assert
}

TEST_F(wrapper_tests_coverage, time_coverage)
{
    // Arrange
    static char timestamp[256];
    // Act
    const char* time = clock_datetime(timestamp, sizeof(timestamp), '-', ' ', ':');

    // Assert
    ASSERT_NE(time, nullptr);
}

TEST_F(wrapper_tests_coverage, getcwd_coverage)
{
    // Arrange
    // Act
    char *wd = getcwd_wrapper(NULL, 0);
    free_wrapper(wd);
    // Assert
}

TEST_F(wrapper_tests_coverage, usleep_coverage)
{
    // Arrange
    // Act
    usleep_wrapper(1);
    // Assert
}

TEST_F(wrapper_tests_coverage, open_close_coverage)
{
    // Arrange
    // Act
    int res = open_wrapper("/nonexistent", 0);
    close_wrapper(res);
    // Assert
}