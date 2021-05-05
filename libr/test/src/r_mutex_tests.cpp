#include <string>
#include "gtest/gtest.h"

#include "r_mutex.h"

extern "C" {
#include "pthread.condition.mock.h"
#include "pthread.mutex.mock.h"
#include "pthread.mock.h"
#include "mem.mock.h"
}

struct _mutex_t
{
    pthread_mutex_t mutex;
};


class thread_pthread_tests : public ::testing::Test
{
protected:
    thread_pthread_tests() = default;

    ~thread_pthread_tests() override = default;

    void SetUp() override
    {
        RESET_FAKE(test_run_function_mock)
        RESET_FAKE(pthread_create);
        RESET_FAKE(pthread_join);

        RESET_FAKE(pthread_cond_init);
        RESET_FAKE(pthread_cond_destroy);
        RESET_FAKE(pthread_cond_wait);
        RESET_FAKE(pthread_cond_signal);

        RESET_FAKE(pthread_mutex_init);
        RESET_FAKE(pthread_mutex_destroy);
        RESET_FAKE(pthread_mutex_lock);
        RESET_FAKE(pthread_mutex_unlock);

        RESET_FAKE(safe_malloc);
        RESET_FAKE(safe_free);

    }

    void TearDown() override
    {
    }

    static int pthread_create_custom_fake( pthread_t *thread, const pthread_attr_t *attr, start_routine_cb start, void *data)
    {
        (void )thread;
        (void)attr;
        start(data);
        return 0;
    }

public:
};


TEST_F(thread_pthread_tests, new_mutex_returns_mutex)
{
    // Arrange
    mutex_t mutex_data;
    safe_malloc_fake.return_val = &mutex_data;

    // Act
    mutex_t* actual = new_mutex();

    //Assert
    ASSERT_EQ(actual, &mutex_data);
    ASSERT_EQ(pthread_mutex_init_fake.call_count, 1);
}

TEST_F(thread_pthread_tests, delete_mutex_deletes_mutex)
{
    // Arrange
    pthread_mutex_t pthread_mutex{};
    mutex_t mutex_data;
    mutex_data.mutex = pthread_mutex;

    // Act
     delete_mutex(&mutex_data);

    //Assert
    ASSERT_EQ(pthread_mutex_destroy_fake.call_count, 1);
    ASSERT_EQ(pthread_mutex_destroy_fake.arg0_val, &mutex_data.mutex);
    ASSERT_EQ(safe_free_fake.call_count, 1);
}

TEST_F(thread_pthread_tests, delete_mutex_does_not_delete_mutex_when_null)
{
    // Arrange
    // Act
    delete_mutex(nullptr);

    //Assert
    ASSERT_EQ(pthread_mutex_destroy_fake.call_count, 0);
    ASSERT_EQ(safe_free_fake.call_count, 0);
}

TEST_F(thread_pthread_tests, mutex_lock_locks_mutex)
{
    // Arrange
    mutex_t mutex_data;

    // Act
    mutex_lock(&mutex_data);

    //Assert
    ASSERT_EQ(pthread_mutex_lock_fake.call_count, 1);
}


TEST_F(thread_pthread_tests, mutex_unlock_unlocks_mutex)
{
    // Arrange
    mutex_t mutex_data;

    // Act
    mutex_unlock(&mutex_data);

    //Assert
    ASSERT_EQ(pthread_mutex_unlock_fake.call_count, 1);
}

