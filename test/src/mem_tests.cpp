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
        RESET_FAKE(free_wrapper);
        RESET_FAKE(malloc_wrapper);
        RESET_FAKE(memset_wrapper);
        RESET_FAKE(realloc_wrapper);
        RESET_FAKE(r_err);
        RESET_FAKE(r_warn);
	}

	void TearDown() override
    {
	}

};

TEST_F(mem_tests, mem_init_coverage)
{
    // Arrange
    // Act
        mem_init(nullptr, nullptr);
    // Assert
}
TEST_F(mem_tests, mem_cleanup_coverage)
{
    // Arrange
    // Act
    mem_cleanup();
    // Assert
}

TEST_F(mem_tests, safe_malloc_returns_NULL_and_logs_warning_when_size_0)
{
    // Arrange
    const int size = 0;

    // Act
    void *ptr = safe_malloc(size, 0);

    // Assert
    ASSERT_EQ(malloc_wrapper_fake.call_count, 0);
    ASSERT_EQ(r_warn_fake.call_count, 1);
    ASSERT_EQ(ptr, nullptr);
}

TEST_F(mem_tests, safe_malloc_returns_NULL_and_logs_error_when_malloc_fails)
{
    // Arrange
    const int size = 10;

    malloc_wrapper_fake.return_val = nullptr;

    // Act
    void *ptr = safe_malloc(size, 0);

    // Assert
    ASSERT_EQ(malloc_wrapper_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(ptr, nullptr);
}

TEST_F(mem_tests, safe_malloc_calls_memset_wrapper_when_zero_flag_set)
{
    // Arrange
    const int size = 10;
    uint8_t buffer[size];
    malloc_wrapper_fake.return_val = &buffer;

    // Act
    void *ptr = safe_malloc( size, 1);

    // Assert
    ASSERT_EQ(malloc_wrapper_fake.call_count, 1);
    ASSERT_EQ(malloc_wrapper_fake.arg0_val, size);
    ASSERT_EQ(memset_wrapper_fake.call_count, 1);
    ASSERT_EQ(memset_wrapper_fake.arg0_val, &buffer);
    ASSERT_EQ(memset_wrapper_fake.arg1_val, 0);
    ASSERT_EQ(memset_wrapper_fake.arg2_val, size);
    ASSERT_EQ(ptr, &buffer);
}

TEST_F(mem_tests, safe_free_frees_ptr_when_ptr)
{
    // Arrange
    const int size = 10;
    void *buffer = malloc(size);
    free_wrapper_fake.custom_fake = free;

    // Act
    safe_free(buffer);

    // Assert
    ASSERT_EQ(r_warn_fake.call_count, 0);
    ASSERT_EQ(free_wrapper_fake.arg0_val, buffer);
    ASSERT_EQ(free_wrapper_fake.call_count, 1);
}

TEST_F(mem_tests, safe_free_warns_when_ptr_NULL)
{
    // Arrange
    // Act
    safe_free(nullptr);

    // Assert
    ASSERT_EQ(r_warn_fake.call_count, 1);
    ASSERT_EQ(free_wrapper_fake.call_count, 0);
}

TEST_F(mem_tests, safe_calloc_calls_with_correct_size)
{
    // Arrange
    const int number = 10;
    int size = sizeof(uint16_t);
    int expected_size = number * size;
    uint16_t buffer[number];
    malloc_wrapper_fake.return_val = &buffer;

    // Act
    void *calloc_buffer = safe_calloc(number, size);

    // Assert
    ASSERT_EQ(malloc_wrapper_fake.call_count, 1);
    ASSERT_EQ(malloc_wrapper_fake.arg0_val, expected_size);
    ASSERT_EQ(memset_wrapper_fake.call_count, 1);
    ASSERT_EQ(memset_wrapper_fake.arg0_val, &buffer);
    ASSERT_EQ(memset_wrapper_fake.arg1_val, 0);
    ASSERT_EQ(memset_wrapper_fake.arg2_val, expected_size);
    ASSERT_EQ(calloc_buffer, &buffer[0]);
}

TEST_F(mem_tests, safe_realloc_warns_when_size_0)
{
    // Arrange
    const int size = 0;
    realloc_wrapper_fake.return_val = nullptr;
    void *pdata = nullptr;
    // Act
    void *realloc_buffer = safe_realloc(pdata, size);

    // Assert
    ASSERT_EQ(realloc_wrapper_fake.call_count, 1);
    ASSERT_EQ(r_warn_fake.call_count, 1);
    ASSERT_EQ(realloc_buffer, nullptr);
}

TEST_F(mem_tests, safe_realloc_errors_when_realloc_returns_NULL)
{
    // Arrange
    const int size = 10;
    realloc_wrapper_fake.return_val = nullptr;
    void *pdata = nullptr;
    // Act
    void *realloc_buffer = safe_realloc(pdata, size);

    // Assert
    ASSERT_EQ(realloc_wrapper_fake.call_count, 1);
    ASSERT_EQ(realloc_wrapper_fake.arg0_val, pdata);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(realloc_buffer, nullptr);
}

TEST_F(mem_tests, safe_realloc_returns_pointer_when_realloc_succeeds)
{
    // Arrange
    const int size = 10;
    uint8_t buffer[size];

    realloc_wrapper_fake.return_val = &buffer;
    void *pdata = &buffer;
    // Act
    void *realloc_buffer = safe_realloc(pdata, size);

    // Assert
    ASSERT_EQ(realloc_wrapper_fake.call_count, 1);
    ASSERT_EQ(realloc_wrapper_fake.arg0_val, pdata);
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(realloc_buffer, pdata);
}

TEST_F(mem_tests, safe_strdup_warns_and_returns_NULL_when_NULL)
{
    // Arrange
    // Act
    char *realloc_buffer = safe_strdup(nullptr);

    // Assert
    ASSERT_EQ(r_warn_fake.call_count, 1);
    ASSERT_EQ(realloc_buffer, nullptr);
}

TEST_F(mem_tests, safe_strdup_when_string_over_1MB_returns_NULL)
{
    // Arrange
    char big_buffer[1024*1024+4] = {0};
    int value = 4;
    memset(big_buffer, value, sizeof(big_buffer));

    // Act
    char *realloc_buffer = safe_strdup(big_buffer);

    // Assert
    ASSERT_EQ(r_warn_fake.call_count, 1);
    ASSERT_EQ(realloc_buffer, nullptr);
}

TEST_F(mem_tests, safe_strdup_when_string_not_over_1MB_allocates_correct_length)
{
    // Arrange
    std::string teststring("teststring");
    const ulong expected_size = teststring.length() + 1;
    std::vector<char> buffer(expected_size, 0);

    malloc_wrapper_fake.return_val = &buffer[0];

    // Act
    char *dup_str = safe_strdup(teststring.c_str());
    std::string actual(dup_str);

    // Assert
    ASSERT_EQ(malloc_wrapper_fake.call_count, 1);
    ASSERT_EQ(malloc_wrapper_fake.arg0_val, expected_size);
    ASSERT_EQ(dup_str, &buffer[0]);
    ASSERT_EQ(actual, teststring);
}
