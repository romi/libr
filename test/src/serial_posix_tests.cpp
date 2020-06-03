#include <string>
#include "gtest/gtest.h"

#include "serial.h"

extern "C" {
#include "fcntl.mock.h"
#include "mutex.mock.h"
#include "mem.mock.h"
}


class serial_posix_tests : public ::testing::Test
{
protected:
    serial_posix_tests() = default;

    ~serial_posix_tests() override = default;

    void SetUp() override
    {
        RESET_FAKE(new_mutex);
        RESET_FAKE(delete_mutex);
        RESET_FAKE(mutex_lock);
        RESET_FAKE(mutex_unlock);

        RESET_FAKE(safe_malloc);
        RESET_FAKE(safe_free);

        RESET_FAKE(open);

    }

    void TearDown() override
    {
    }

public:
};


TEST_F(serial_posix_tests, new_serial)
{
    // Arrange
    safe_malloc_fake.return_val = nullptr;

    // Act

    //Assert
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
}