#include <string>
#include "gtest/gtest.h"

#include "mem.h"

extern "C" {
#include "log.mock.h"
#include "osw.mock.h"
}


class mem_tests : public ::testing::Test
{
protected:
	mem_tests()
    {
	}

	virtual ~mem_tests()
    {
	}

	virtual void SetUp() 
    {
        RESET_FAKE(malloc_w);
        RESET_FAKE(r_err);
	}

	virtual void TearDown() 
    {
	}

};

TEST_F(mem_tests, safe_malloc_returns_NULL_and_logs_when_malloc_fails)
{
    // Arrange
    malloc_w_fake.return_val = NULL;

    // Act
    void *ptr = safe_malloc(10, 0);

    //AssertNULL
    ASSERT_EQ(malloc_w_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(ptr, nullptr);
}

