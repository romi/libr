#include <string>
#include "gtest/gtest.h"

#include "thread.h"

extern "C" {
#include "pthread.condition.mock.h"
#include "pthread.mutex.mock.h"
#include "pthread.mock.h"
#include "mem.mock.h"
}

struct _thread_t {
    pthread_t thread;
    thread_run_t run;
    void *data;
    int autodelete;
};

struct _mutex_t
{
    pthread_mutex_t mutex;
};

struct _condition_t
{
    pthread_cond_t cond;
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


//TEST_F(thread_pthread_tests, thread_pthread_create_fails_returns_NULL)
//{
//    // Arrange
//    safe_malloc_fake.return_val = nullptr;
//
//    // Act
//    thread_t *test_thread = new_thread(test_run_function_mock, nullptr, 0, 0);
//
//    //Assert
//    ASSERT_EQ(nullptr, test_thread);
//
//    ASSERT_EQ(safe_malloc_fake.call_count, 1);
//}

TEST_F(thread_pthread_tests, thread_pthread_create_fails_deletes_thread_data)
{
    // Arrange
    thread_t thread_data;
    safe_malloc_fake.return_val = &thread_data;
    pthread_create_fake.return_val = -1;

    // Act
    thread_t *test_thread = new_thread(test_run_function_mock, nullptr, 0, 0);

    //Assert
    ASSERT_EQ(nullptr, test_thread);
    ASSERT_EQ(safe_free_fake.call_count, 1);
    ASSERT_EQ(safe_free_fake.arg0_val, &thread_data);
    ASSERT_EQ(test_run_function_mock_fake.call_count, 0);
}


TEST_F(thread_pthread_tests, thread_pthread_create_succeeds_returns_thread)
{
    // Arrange
    thread_t thread_data;
    safe_malloc_fake.return_val = &thread_data;
    pthread_create_fake.return_val = 0;

    // Act
    thread_t *test_thread = new_thread(test_run_function_mock, nullptr, 0, 0);

    //Assert
    ASSERT_EQ(&thread_data, test_thread);
    ASSERT_EQ(safe_free_fake.call_count, 0);
}

TEST_F(thread_pthread_tests, thread_pthread_sets_correct_data_in_thread)
{
    // Arrange
    thread_t thread_data;
    safe_malloc_fake.return_val = &thread_data;
    pthread_create_fake.return_val = 0;

    int data = 10;
    int autodelete = 1;

    // Act
    thread_t *test_thread = new_thread(test_run_function_mock, &data, 0, autodelete);

    //Assert
    ASSERT_EQ(&thread_data, test_thread);
    ASSERT_EQ(safe_free_fake.call_count, 0);
    ASSERT_EQ(thread_data.data, &data);
    ASSERT_EQ(thread_data.autodelete, autodelete);
    ASSERT_EQ(thread_data.run, test_run_function_mock);
}

TEST_F(thread_pthread_tests, thread_run_runs_correct_function)
{
    // Arrange
    thread_t thread_data;
    safe_malloc_fake.return_val = &thread_data;
    pthread_create_fake.return_val = 0;
    pthread_create_fake.custom_fake = thread_pthread_tests::pthread_create_custom_fake;

    int data = 10;
    int autodelete = 0;

    // Act
    thread_t *test_thread = new_thread(test_run_function_mock, &data, 0, autodelete);

    //Assert
    ASSERT_EQ(&thread_data, test_thread);
    ASSERT_EQ(safe_free_fake.call_count, 0);
    ASSERT_EQ(thread_data.data, &data);
    ASSERT_EQ(thread_data.autodelete, autodelete);
    ASSERT_EQ(thread_data.run, test_run_function_mock);
    ASSERT_EQ(test_run_function_mock_fake.call_count, 1);
}

TEST_F(thread_pthread_tests, thread_run_with_auto_delete_deletes)
{
    // Arrange
    thread_t thread_data;
    safe_malloc_fake.return_val = &thread_data;
    pthread_create_fake.return_val = 0;
    pthread_create_fake.custom_fake = thread_pthread_tests::pthread_create_custom_fake;

    int data = 10;
    int autodelete = 1;

    // Act
    thread_t *test_thread = new_thread(test_run_function_mock, &data, 0, autodelete);

    //Assert
    ASSERT_EQ(&thread_data, test_thread);
    ASSERT_EQ(safe_free_fake.call_count, 1);
    ASSERT_EQ(thread_data.data, &data);
    ASSERT_EQ(thread_data.autodelete, autodelete);
    ASSERT_EQ(thread_data.run, test_run_function_mock);
    ASSERT_EQ(test_run_function_mock_fake.call_count, 1);
}

TEST_F(thread_pthread_tests, thread_join_calls_join)
{
    // Arrange
    thread_t thread_data;
    pthread_t pthread_data = 0;
    thread_data.thread = pthread_data;
    pthread_join_fake.return_val = 10;

    // Act
    int actual = thread_join(&thread_data);

    //Assert
    ASSERT_EQ(actual, 10);
    ASSERT_EQ(pthread_join_fake.call_count, 1);
    ASSERT_EQ(pthread_join_fake.arg0_val, pthread_data);
}


//TEST_F(thread_pthread_tests, new_mutex_fails_returns_NULL)
//{
//    // Arrange
//    safe_malloc_fake.return_val = nullptr;
//
//    // Act
//    mutex_t* actual = new_mutex();
//
//    //Assert
//    ASSERT_EQ(actual, nullptr);
//    ASSERT_EQ(pthread_mutex_init_fake.call_count, 0);
//}

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
    pthread_mutex_t pthread_mutex  = PTHREAD_MUTEX_INITIALIZER;

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

//TEST_F(thread_pthread_tests, new_condition_fails_returns_NULL)
//{
//    // Arrange
//    safe_malloc_fake.return_val = nullptr;
//
//    // Act
//    condition_t* actual = new_condition();
//
//    //Assert
//    ASSERT_EQ(actual, nullptr);
//    ASSERT_EQ(pthread_cond_init_fake.call_count, 0);
//}

TEST_F(thread_pthread_tests, new_condition_returns_condition)
{
    // Arrange
    condition_t condition_data;
    safe_malloc_fake.return_val = &condition_data;

    // Act
    condition_t* actual = new_condition();

    //Assert
    ASSERT_EQ(actual, &condition_data);
    ASSERT_EQ(pthread_cond_init_fake.call_count, 1);
}

TEST_F(thread_pthread_tests, delete_condition_deletes_condition)
{
    // Arrange
    pthread_cond_t pthread_condition = PTHREAD_COND_INITIALIZER;

    condition_t condition_data;
    condition_data.cond = pthread_condition;

    // Act
    delete_condition(&condition_data);

    //Assert
    ASSERT_EQ(pthread_cond_destroy_fake.call_count, 1);
    ASSERT_EQ(pthread_cond_destroy_fake.arg0_val, &condition_data.cond);
    ASSERT_EQ(safe_free_fake.call_count, 1);
}

TEST_F(thread_pthread_tests, delete_condition_does_not_delete_condition_when_null)
{
    // Arrange
    // Act
    delete_condition(nullptr);

    //Assert
    ASSERT_EQ(pthread_cond_destroy_fake.call_count, 0);
    ASSERT_EQ(safe_free_fake.call_count, 0);
}

TEST_F(thread_pthread_tests, condition_wait_calls_pthread_cond_wait)
{
    // Arrange
    condition_t condition_data;
    mutex_t mutex_data;

    // Act
    condition_wait(&condition_data, &mutex_data);

    //Assert
    ASSERT_EQ(pthread_cond_wait_fake.call_count, 1);
}


TEST_F(thread_pthread_tests, condition_signal_calls_pthread_cond_signal)
{
    // Arrange
    condition_t condition_data;

    // Act
    condition_signal(&condition_data);

    //Assert
    ASSERT_EQ(pthread_cond_signal_fake.call_count, 1);
}
