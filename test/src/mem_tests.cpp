#include <string>
#include "gtest/gtest.h"

#include "mem.h"

extern "C" {
#include "log.mock.h"
#include "os_wrapper.mock.h"
}


class mem_tests : public ::testing::Test
{
protected:
	mem_tests() = default;

	~mem_tests() override = default;

	void SetUp() override
    {
        RESET_FAKE(malloc_wrapper);
        RESET_FAKE(r_err);
	}

	void TearDown() override
    {
	}

};

TEST_F(mem_tests, safe_malloc_returns_NULL_and_logs_when_malloc_fails)
{
    // Arrange
    malloc_wrapper_fake.return_val = nullptr;

    // Act
    void *ptr = safe_malloc(10, 0);

    //AssertNULL
    ASSERT_EQ(malloc_wrapper_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(ptr, nullptr);
}

