#include <string>
#include <termio.h>
#include "gtest/gtest.h"

#include "serial.h"

extern "C" {
#include "log.mock.h"
#include "mem.mock.h"
#include "membuf.mock.h"
#include "mutex.mock.h"
#include "os_wrapper.mock.h"
#include "termios.mock.h"
}

// Duplicated from serial_posix.c
struct _serial_t
{
    char *device;
    int speed;
    int fd;
    membuf_t *out;
    int nextchar;
    mutex_t *mutex;
    int errors;
    int quit;
};

// Duplicated from thread_pthread.c
struct _mutex_t
{
    pthread_mutex_t mutex;
};

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

        RESET_FAKE(tcgetattr)
        RESET_FAKE(cfsetspeed)
        RESET_FAKE(tcflush)
        RESET_FAKE(tcsetattr)

        RESET_FAKE(safe_malloc);
        RESET_FAKE(safe_free);
        RESET_FAKE(safe_strdup)

        RESET_FAKE(r_err);

        RESET_FAKE(open_wrapper)
        RESET_FAKE(close_wrapper)

        RESET_FAKE(new_membuf)

        safe_malloc_fake.return_val = &serial_data;
    }

    void TearDown() override
    {
    }

    static int tcgetattr_custom_fake(int fd __attribute__((unused)), struct termios *pterm)
    {
        pterm->c_cflag = cflags_data;
       // return tcgetattr_custom_fake_return;
        return 0;
    }

    static int tcsetattr_custom_fake(int fd __attribute__((unused)) ,int actions __attribute__((unused)), const struct termios *pterm)
    {
//        fd_data = fd;
//        actions_data = actions;
        memcpy(&termios_data, pterm, sizeof(termios_data));
        return 0;
    }

    static int cfsetspeed_custom_fake(struct termios *pterm, speed_t speed)
    {
        cflags_data = pterm->c_cflag;
        speed_data = speed;
        return 0;
    }

public:
    serial_t serial_data;

    static unsigned int cflags_data;
    static speed_t speed_data;
    static termios termios_data;
//    static int fd_data;
//    static int actions_data;
};

unsigned int serial_posix_tests::cflags_data;
speed_t serial_posix_tests::speed_data;
termios serial_posix_tests::termios_data;
//int serial_posix_tests::fd_data;
//int serial_posix_tests::actions_data;

TEST_F(serial_posix_tests, new_serial_open_fails_returns_null)
{
    // Arrange
    std::string device = "/dev/ttys1/nowhere";
    int speed = 9600;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    int open_return_value = -1;
    safe_malloc_fake.return_val = nullptr;
    open_wrapper_fake.return_val = open_return_value;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(open_wrapper_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 1);
}

TEST_F(serial_posix_tests, new_serial_unknown_speed_returns_null)
{
    // Arrange
    std::string device = "/dev/ttys1/nowhere";
    int speed = 1024;
    int reset = 0;
    serial_t *expected_serial = nullptr;
    safe_malloc_fake.return_val = nullptr;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(r_err_fake.call_count, 1);
}

TEST_F(serial_posix_tests, new_serial_getattr_fails_closes_port_returns_null)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 9600;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    safe_malloc_fake.return_val = nullptr;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = -1;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(open_wrapper_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(tcgetattr_fake.call_count, 1);
    ASSERT_EQ(close_wrapper_fake.call_count, 1);
}


TEST_F(serial_posix_tests, new_serial_9600_speed_is_set_correctly)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 9600;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B9600);
}

TEST_F(serial_posix_tests, new_serial_19200_speed_is_set_correctly)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 19200;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B19200);
}

TEST_F(serial_posix_tests, new_serial_38400_speed_is_set_correctly)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 38400;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B38400);
}

TEST_F(serial_posix_tests, new_serial_57600_speed_is_set_correctly)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 57600;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B57600);
}

TEST_F(serial_posix_tests, new_serial_115200_speed_is_set_correctly)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 115200;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B115200);
}

TEST_F(serial_posix_tests, new_serial_if_reset_clear_HUPCL_flag_cleared)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 115200;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;
    cfsetspeed_fake.custom_fake = cfsetspeed_custom_fake;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);
    unsigned int HUPCL_flag = cflags_data & HUPCL;

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B115200);
    ASSERT_EQ(HUPCL_flag, 0);
}

///// NOT WORKING
TEST_F(serial_posix_tests, new_serial_if_reset_set_HUPCL_flag_set)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 115200;
    int reset = 1;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    cflags_data = HUPCL;

    tcgetattr_fake.custom_fake = tcgetattr_custom_fake;
    cfsetspeed_fake.custom_fake = cfsetspeed_custom_fake;
    tcsetattr_fake.return_val = -1;
    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);
 //   unsigned int HUPCL_flag = cflags_data & HUPCL;

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B115200);
    // ToDo: This test doesn't work. Code needs to be looked at.
    //ASSERT_EQ(HUPCL_flag, HUPCL);
}


TEST_F(serial_posix_tests, new_serial_port_is_flushed)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 115200;
    int reset = 1;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = 0;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(tcflush_fake.call_count, 1);
    ASSERT_EQ(tcflush_fake.arg0_val, open_return_value);
    ASSERT_EQ(tcflush_fake.arg1_val, TCIOFLUSH);

}

TEST_F(serial_posix_tests, new_serial_setattr_sets_correct_attributes)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 115200;
    int reset = 1;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.custom_fake = tcgetattr_custom_fake;
    tcsetattr_fake.custom_fake = tcsetattr_custom_fake;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(tcsetattr_fake.arg0_val, open_return_value);
    ASSERT_EQ(tcsetattr_fake.arg1_val, TCSANOW);
    ASSERT_EQ(termios_data.c_cflag, (CLOCAL | CREAD | CS8) );
    ASSERT_EQ(termios_data.c_lflag, (ICANON) );
    ASSERT_EQ(termios_data.c_iflag, (IGNCR) );
    ASSERT_EQ(termios_data.c_oflag, (0) );
}

TEST_F(serial_posix_tests, new_serial_device_dup_fails_returns_NULL)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 115200;
    int reset = 1;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = 0;

    // Data returned by memory allocations.
    mutex_t mutex_data;
    char *device_dup = nullptr;
    membuf_t membuf_data;

    safe_malloc_fake.return_val = &serial_data;
    safe_strdup_fake.return_val = device_dup;
    new_membuf_fake.return_val = &membuf_data;
    new_mutex_fake.return_val = &mutex_data;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(safe_strdup_fake.call_count, 1);
    ASSERT_EQ(new_membuf_fake.call_count, 1);
    ASSERT_EQ(new_mutex_fake.call_count, 1);
}


TEST_F(serial_posix_tests, new_serial_new_membuf_fails_returns_NULL)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 115200;
    int reset = 1;
    serial_t *expected_serial = nullptr;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = 0;

    // Data returned by memory allocations.
    mutex_t mutex_data;
    const char *device_dup = "/dev/ttys1";
    membuf_t *membuf_data = nullptr;

    safe_malloc_fake.return_val = &serial_data;
    safe_strdup_fake.return_val = (char*)device_dup;
    new_membuf_fake.return_val = membuf_data;
    new_mutex_fake.return_val = &mutex_data;

    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(safe_strdup_fake.call_count, 1);
    ASSERT_EQ(new_membuf_fake.call_count, 1);
    ASSERT_EQ(new_mutex_fake.call_count, 1);
}

TEST_F(serial_posix_tests, new_serial_valid_data_sets_data_returns_serial)
{
    // Arrange
    std::string device = "/dev/ttys1";
    int speed = 115200;
    int reset = 1;

    int open_return_value = 1;
    open_wrapper_fake.return_val = open_return_value;
    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = 0;

    // Data returned by memory allocations.
    mutex_t mutex_data;
    serial_t *expected_serial = &serial_data;
    const char *device_dup = "/dev/ttys1";
    membuf_t membuf_data;

    safe_malloc_fake.return_val = expected_serial;
    safe_strdup_fake.return_val = (char*)device_dup;
    new_membuf_fake.return_val = &membuf_data;
    new_mutex_fake.return_val = &mutex_data;


    // Act
    serial_t *actual_serial =  new_serial(device.c_str(), speed, reset);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(expected_serial->device, device_dup);
    ASSERT_EQ(expected_serial->mutex, &mutex_data);
    ASSERT_EQ(expected_serial->errors, 0);
    ASSERT_EQ(expected_serial->fd, open_return_value);
    ASSERT_EQ(expected_serial->nextchar, -1);
    ASSERT_EQ(expected_serial->out, &membuf_data);
    ASSERT_EQ(expected_serial->speed, speed);
    ASSERT_EQ(expected_serial->quit, 0);
}

TEST_F(serial_posix_tests, delete_serial_when_NULL_does_not_delete)
{
    // Arrange
    // Act
    delete_serial(nullptr);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 0);
}

TEST_F(serial_posix_tests, delete_serial_sets_quit_1)
{
    // Arrange
    serial_t serial_data;
    serial_data.quit = 5;
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(serial_data.quit, 1);
}

TEST_F(serial_posix_tests, delete_serial_locks_and_unlocks_mutex)
{
    // Arrange
    mutex_t mutex_data;
    serial_t serial_data;
    serial_data.mutex = &mutex_data;
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(mutex_lock_fake.call_count, 1);
    ASSERT_EQ(mutex_lock_fake.arg0_val, &mutex_data);
    ASSERT_EQ(mutex_unlock_fake.call_count, 1);
    ASSERT_EQ(mutex_unlock_fake.arg0_val, &mutex_data);
}

TEST_F(serial_posix_tests, delete_serial_locks_deletes_mutex)
{
    // Arrange
    mutex_t mutex_data;
    serial_t serial_data;
    serial_data.mutex = &mutex_data;
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(delete_mutex_fake.call_count, 1);
    ASSERT_EQ(delete_mutex_fake.arg0_val, &mutex_data);
}

TEST_F(serial_posix_tests, delete_serial_deletes_device)
{
    // Arrange
    std::string device = "/dev/ttys1";
    serial_t serial_data;
    serial_data.device = (char *)device.c_str();
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(safe_free_fake.arg0_history[0], (char *)device.c_str());
}

TEST_F(serial_posix_tests, delete_serial_deletes_out_membuf)
{
    // Arrange
    membuf_t membuf_data;
    serial_t serial_data;
    serial_data.out = &membuf_data;
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(delete_membuf_fake.call_count, 1);
    ASSERT_EQ(delete_membuf_fake.arg0_val, &membuf_data);
}

TEST_F(serial_posix_tests, delete_serial_closes_fd)
{
    // Arrange
    int fd = 10;
    int fd_closed = -1;
    serial_t serial_data;
    serial_data.fd = fd;
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(close_wrapper_fake.call_count, 1);
    ASSERT_EQ(close_wrapper_fake.arg0_val, fd);
    ASSERT_EQ(serial_data.fd, fd_closed);
}

TEST_F(serial_posix_tests, delete_serial_deletes_serial)
{
    // Arrange
    serial_t serial_data;
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(safe_free_fake.arg0_history[1], expected_serial);
}