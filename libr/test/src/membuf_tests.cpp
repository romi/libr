#include <string>
#include <vector>
#include <strings.h>
#include <r.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "membuf.h"

extern "C" {
#include "log.mock.h"
#include "mem.mock.h"
#include "mutex.mock.h"
}


class membuf_tests : public ::testing::Test
{
protected:
	membuf_tests() = default;

	~membuf_tests() override = default;

	void SetUp() override
    {
        RESET_FAKE(delete_mutex);
        RESET_FAKE(new_mutex);
        RESET_FAKE(mutex_lock)
        RESET_FAKE(mutex_unlock)
        RESET_FAKE(safe_malloc);
        RESET_FAKE(safe_free);
        RESET_FAKE(safe_realloc);
        RESET_FAKE(r_err);
        RESET_FAKE(r_err);
        RESET_FAKE(r_warn);
	}

	void TearDown() override
    {
	}

	void SetupMemBufferAndMutex(char* buffer, size_t length, size_t index)
    {
        memset(&membuffer, 0, sizeof(membuffer));
        char fakemutex = 'p';
        mutex_t *pmutex = (mutex_t *)&fakemutex;

        membuffer.mutex = pmutex;
        membuffer.buffer = buffer;
        membuffer.length = length;
        membuffer.index = index;
    }

    _membuf_t membuffer;
};

TEST_F(membuf_tests, r_init_init__cleanup_COVERAGE)
{
    // Arrange
    // Act
    int actual = r_init(NULL, NULL);
    r_cleanup();
    // Assert
    ASSERT_EQ(actual, 0);
}


TEST_F(membuf_tests, new_membuf_when_new_succeeds_creates_mutex_returns_buffer)
{
    // Arrange
    safe_malloc_fake.return_val = &membuffer;

    char fakemutex = 'p';
    mutex_t *pmutex = (mutex_t *)&fakemutex;
    new_mutex_fake.return_val = pmutex;

    // Act
    membuf_t *actual = new_membuf();

    // Assert
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(new_mutex_fake.call_count, 1);
    ASSERT_EQ(actual, &membuffer);
    ASSERT_EQ(membuffer.mutex, pmutex);
}

TEST_F(membuf_tests, delete_membuf_when_mutex_not_null_deletes_mutex)
{
    // Arrange
    SetupMemBufferAndMutex(nullptr, 0, 0);

    // Act
    delete_membuf(&membuffer);

    // Assert
    ASSERT_EQ(delete_mutex_fake.call_count, 1);
    ASSERT_EQ(delete_mutex_fake.arg0_val, membuffer.mutex);
    ASSERT_EQ(safe_free_fake.call_count, 2);
}

TEST_F(membuf_tests, delete_membuf_when_buffer_not_null_deletes_buffer_and_membuf)
{
    // Arrange
    const int buffer_size = 10;
    char buffer[buffer_size] = {0};
    SetupMemBufferAndMutex(buffer, 0, 0);

    // Act
    delete_membuf(&membuffer);

    // Assert
    ASSERT_EQ(delete_mutex_fake.call_count, 1);
    ASSERT_EQ(safe_free_fake.arg0_history[0], buffer);
    ASSERT_EQ(safe_free_fake.arg0_history[1], &membuffer);
    ASSERT_EQ(safe_free_fake.call_count, 2);
}

TEST_F(membuf_tests, membuf_put_when_membuf_empty_grows_by_128_and_puts_c)
{
    // Arrange
    int expected_length = 128;
    char expected_char = 'a';

    const int buffer_size = 10;
    char buffer[buffer_size] = {0};
    SetupMemBufferAndMutex(buffer, 0, 0);

    safe_realloc_fake.return_val = buffer;

    // Act
    membuf_put(&membuffer, expected_char);

    // Assert
    ASSERT_EQ(safe_realloc_fake.call_count, 1);
    ASSERT_EQ(safe_realloc_fake.arg1_val, expected_length);
    ASSERT_EQ(membuffer.buffer[0], expected_char);
    ASSERT_EQ(membuffer.length, expected_length);
}

TEST_F(membuf_tests, membuf_put_when_membuf_not_empty_and_full_grows_by_2x_and_puts_c)
{
    // Arrange
    char buffer[2] = {'a', 0};
    char expected_char = 'b';
    int expected_length = 2;

    SetupMemBufferAndMutex(buffer, 1, 1);

    safe_realloc_fake.return_val = buffer;

    // Act
    membuf_put(&membuffer, expected_char);

    // Assert
    ASSERT_EQ(safe_realloc_fake.call_count, 1);
    ASSERT_EQ(safe_realloc_fake.arg1_val, expected_length);
    ASSERT_EQ(membuffer.buffer, buffer);
    ASSERT_EQ(membuffer.buffer[0], 'a');
    ASSERT_EQ(membuffer.buffer[1], expected_char);
    ASSERT_EQ(membuffer.length, expected_length);
    ASSERT_EQ(membuffer.index, expected_length);
}

TEST_F(membuf_tests, membuf_lock_calls_mutex_lock_with_correct_parameter)
{
    // Arrange
    memset(&membuffer, 0, sizeof(membuffer));
    char fakemutex = 'p';
    mutex_t *pmutex = (mutex_t *)&fakemutex;
    membuffer.mutex = pmutex;

    // Act
    membuf_lock(&membuffer);

    // Assert
    ASSERT_EQ(mutex_lock_fake.call_count, 1);
    ASSERT_EQ(mutex_lock_fake.arg0_val, pmutex);
}

TEST_F(membuf_tests, membuf_unlock_calls_mutex_unlock_with_correct_parameter)
{
    // Arrange
    memset(&membuffer, 0, sizeof(membuffer));
    char fakemutex = 'p';
    mutex_t *pmutex = (mutex_t *)&fakemutex;
    membuffer.mutex = pmutex;

    // Act
    membuf_unlock(&membuffer);

    // Assert
    ASSERT_EQ(mutex_unlock_fake.call_count, 1);
    ASSERT_EQ(mutex_unlock_fake.arg0_val, pmutex);
}

TEST_F(membuf_tests, membuf_mutex_returns_correct_value)
{
    // Arrange
    memset(&membuffer, 0, sizeof(membuffer));
    char fakemutex = 'p';
    mutex_t *pmutex = (mutex_t *)&fakemutex;
    membuffer.mutex = pmutex;

    // Act
    mutex_t *actual = membuf_mutex(&membuffer);

    // Assert
    ASSERT_EQ(actual, pmutex);
}

TEST_F(membuf_tests, membuf_append_size_ok_does_not_call_grow)
{
    // Arrange
    const int dest_buffer_size = 8;
    char dest_buffer[dest_buffer_size] = {0};

    const int src_buffer_size = 4;
    char src_buffer[src_buffer_size] = {'a', 'a', 'a', 'a'};

    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, 0);

    // Act
    membuf_append(&membuffer, src_buffer, src_buffer_size);

    // Assert
    ASSERT_EQ(safe_realloc_fake.call_count, 0);
}

TEST_F(membuf_tests, membuf_append_size_just_too_small_calls_grow_once)
{
    // Arrange
    const int dest_buffer_size = 2;
    char dest_buffer[dest_buffer_size] = {0};

    const int src_buffer_size = 4;
    char src_buffer[src_buffer_size] = {'a', 'a', 'a', 'a'};

    const int grown_buffer_size = 4;
    char grown_buffer[grown_buffer_size];
    safe_realloc_fake.return_val = grown_buffer;

    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, 0);

    // Act
    membuf_append(&membuffer, src_buffer, src_buffer_size);

    // Assert
    ASSERT_EQ(safe_realloc_fake.call_count, 1);
    ASSERT_EQ(safe_realloc_fake.arg0_val, dest_buffer);
    ASSERT_EQ(safe_realloc_fake.arg1_val, dest_buffer_size*2);
}

TEST_F(membuf_tests, membuf_append_calls_grow_correct_number_times)
{
    // Arrange
    const int dest_buffer_size = 2;
    char dest_buffer[dest_buffer_size] = {0};

    const int src_buffer_size = 16;
    char src_buffer[src_buffer_size];
    memset(src_buffer, 'a', sizeof(src_buffer));

    const int grown_buffer_size = 16;
    char grown_buffer[grown_buffer_size];

    safe_realloc_fake.return_val = grown_buffer;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, 0);

    // Act
    membuf_append(&membuffer, src_buffer, src_buffer_size);
    std::vector<char> grown_vector(grown_buffer, grown_buffer + grown_buffer_size);
    std::vector<char> src_vector(src_buffer, src_buffer + src_buffer_size);

    // Assert
    ASSERT_EQ(safe_realloc_fake.call_count, 3);
    ASSERT_EQ(safe_realloc_fake.arg0_history[0], dest_buffer);
    ASSERT_EQ(safe_realloc_fake.arg0_history[1], grown_buffer);
    ASSERT_EQ(safe_realloc_fake.arg0_history[2], grown_buffer);
    ASSERT_EQ(safe_realloc_fake.arg1_history[0], dest_buffer_size*2);
    ASSERT_EQ(safe_realloc_fake.arg1_history[1], (dest_buffer_size*2) * 2);
    ASSERT_EQ(safe_realloc_fake.arg1_history[2], ((dest_buffer_size*2) * 2) * 2);
    ASSERT_EQ(src_vector, grown_vector);
}

TEST_F(membuf_tests, membuf_append_zero_appends_0)
{
    // Arrange
    const int dest_buffer_size = 4;
    char dest_buffer[dest_buffer_size] = {1, 1, 1, 1};
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, 3);

    // Act
    membuf_append_zero(&membuffer);

    // Assert
    ASSERT_EQ(dest_buffer[dest_buffer_size-1], 0);
}

TEST_F(membuf_tests, membuf_append_valid_string_appends_string)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, 0);
    std::string expected_string = "teststring";

    // Act
    membuf_append_str(&membuffer, expected_string.c_str());
    std::string actual_string(dest_buffer);

    // Assert
    ASSERT_EQ(actual_string, expected_string);
}

TEST_F(membuf_tests, membuf_append_string_greater_32k_truncates_string)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size] = {2, 2, 2, 2};
    memset(&dest_buffer, 0, sizeof(dest_buffer));
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, 0);

    const int large_string_size = (1024 * 32 + 2);
    char large_string[large_string_size];
    memset(large_string, 1, large_string_size);
    large_string[large_string_size -1] = 0;

    const int large_buffer_size = 1024 * 32 + 8;
    char large_buffer[large_buffer_size];

    safe_realloc_fake.return_val = large_buffer;

    std::string expected_string(dest_buffer);
    expected_string.append(large_string, 1024 * 32);
    
    // Act
    membuf_append_str(&membuffer, large_string);
    std::string actual_string(membuf_data(&membuffer),
                              membuf_len(&membuffer));

    // Assert
    ASSERT_EQ(actual_string, expected_string);
}

TEST_F(membuf_tests, membuf_clear_sets_index_0_clears_memory)
{
    // Arrange
    const int dest_buffer_size = 8;
    char dest_buffer[dest_buffer_size] = {1, 1, 1, 1, 1, 1, 1, 1};
    memset(&dest_buffer, 0, sizeof(dest_buffer));
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, 4);

    // Act
    membuf_clear(&membuffer);

    // Assert
    ASSERT_EQ(membuffer.index, 0);
    ASSERT_THAT(dest_buffer, ::testing::ElementsAre(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(membuf_tests, membuf_data_returns_correct_value)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, 4);

    // Act
    char *actual = membuf_data(&membuffer);

    // Assert
    ASSERT_EQ(actual, dest_buffer);
}

TEST_F(membuf_tests, membuf_len_returns_correct_value)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));
    size_t expected_index = 4;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, expected_index);

    // Act
    size_t actual = membuf_len(&membuffer);

    // Assert
    ASSERT_EQ(actual, expected_index);
}

TEST_F(membuf_tests, membuf_set_len_does_not_set_legth_when_new_length_too_long)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));
    size_t expected_index = 4;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, expected_index);

    // Act
    membuf_set_len(&membuffer, dest_buffer_size+1);

    // Assert
    ASSERT_EQ(membuffer.index, expected_index);
}

TEST_F(membuf_tests, membuf_set_len_set_length_when_length_valid)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));
    size_t length = 4;
    size_t expected_index = 10;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, length);

    // Act
    membuf_set_len(&membuffer, expected_index);

    // Assert
    ASSERT_EQ(membuffer.index, expected_index);
}

TEST_F(membuf_tests, membuf_availible_returns_correct_value)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));

    size_t index = 4;
    size_t expected_available = dest_buffer_size - index;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, index);

    // Act
    size_t actual = membuf_available(&membuffer);

    // Assert
    ASSERT_EQ(actual, expected_available);
}

TEST_F(membuf_tests, membuf_size_returns_correct_value)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));

    size_t index = 4;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, index);

    // Act
    size_t actual = membuf_size(&membuffer);

    // Assert
    ASSERT_EQ(actual, dest_buffer_size);
}

TEST_F(membuf_tests, membuf_assure_when_grow_succeeds_returns_correct_value)
{
    // Arrange
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));

    size_t index = 4;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, index);

    const int grown_buffer_size = 32;
    char grown_buffer[grown_buffer_size];

    safe_realloc_fake.return_val = grown_buffer;

    // Act
    membuf_assure(&membuffer, 18);

    // Assert
    ASSERT_EQ(membuffer.length, grown_buffer_size);
}



TEST_F(membuf_tests, membuf_printf_with_correct_parameters_prints_to_buffer)
{
    // Arrange
    std::string expected_string = "integer 10";
    const int dest_buffer_size = 16;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));

    size_t index = 0;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, index);

    const int grown_buffer_size = 32;
    char grown_buffer[grown_buffer_size];

    safe_realloc_fake.return_val = grown_buffer;

    // Act
    int actual = membuf_printf(&membuffer, "%s %d", "integer", 10);
    std::string actual_string(dest_buffer);

    // Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(membuffer.length, dest_buffer_size);
    ASSERT_EQ(actual_string, expected_string);
}

TEST_F(membuf_tests, membuf_printf_buffer_grows_returns_correct_value)
{
    // Arrange
    std::string expected_string = "integer";
    const int dest_buffer_size = 4;
    char dest_buffer[dest_buffer_size];
    memset(&dest_buffer, 0, sizeof(dest_buffer));

    size_t index = 0;
    SetupMemBufferAndMutex(dest_buffer, dest_buffer_size, index);

    const int grown_buffer_size = 8;
    char grown_buffer[grown_buffer_size];

    safe_realloc_fake.return_val = grown_buffer;

    // Act
    int actual = membuf_printf(&membuffer, "%s", "integer");
    std::string actual_string(grown_buffer);

    // Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(membuffer.length, grown_buffer_size);
    ASSERT_EQ(actual_string, expected_string);
}
