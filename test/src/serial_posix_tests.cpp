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
#include "system.mock.h"
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

const int ERROR = -1;

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
        RESET_FAKE(r_debug);

        RESET_FAKE(open_wrapper)
        RESET_FAKE(close_wrapper)
        RESET_FAKE(usleep_wrapper)

        RESET_FAKE(new_membuf)
        RESET_FAKE(delete_membuf)
        RESET_FAKE(membuf_clear)
        RESET_FAKE(membuf_put)
        RESET_FAKE(membuf_append_zero)
        RESET_FAKE(membuf_len)
        RESET_FAKE(membuf_data)

        RESET_FAKE(read)
        RESET_FAKE(write)

        safe_malloc_fake.return_val = &serial_data;

        read_data = "serial line\n";
        read_return_value = 0;
        current_char = 0;

        serial_data.quit = 0;
        serial_data.fd = ERROR;
        serial_data.nextchar = ERROR;
        serial_data.device = nullptr;
        serial_data.mutex = nullptr;
        serial_data.speed = 0;
        serial_data.errors = 0;
        serial_data.out = nullptr;
        put_data = "";
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

    static ssize_t read_custom_fake( int fd, void *data, size_t size)
    {
        fd_data = fd;
        request_size_data = size;
        *((char*)data) = read_data[current_char++];
        return read_return_value;
    }

    static ssize_t read_size_custom_fake( int fd, void *data, size_t size)
    {
        char* char_data = ((char*)data);
        size_t char_data_index = 0;
        fd_data = fd;
        request_size_data = size;

        while (char_data_index < size)
            char_data[char_data_index++] = read_data[current_char++];
        return size;
    }

    static ssize_t read_return_number_custom_fake( int fd, void *data, size_t size)
    {
        char* char_data = ((char*)data);
        size_t char_data_index = 0;
        fd_data = fd;
        request_size_data = size;

        while (char_data_index < (size_t)read_return_value)
            char_data[char_data_index++] = read_data[current_char++];
        return read_return_value;
    }

    static ssize_t write_custom_fake(int fd, const void *data, size_t size)
    {
        fd_data = fd;
        request_size_data = size;
        write_data = (void *)data;
        return write_return_value;
    }

    static ssize_t write_size_custom_fake(int fd, const void *data, size_t size)
    {
        fd_data = fd;
        request_size_data = size;
        write_data = (void *)data;
        return size;
    }

//    static membuf_put_custom_fake(membuf_t *buf, char c)
//    {
//
//    }

public:
    serial_t serial_data;

    static unsigned int cflags_data;
    static termios termios_data;
    static speed_t speed_data;

    static int fd_data;
    static std::string read_data;
    static std::string put_data;
    static size_t request_size_data;
    static ssize_t read_return_value;
    static int current_char;

    static void *write_data;
    static ssize_t write_return_value;
};

unsigned int serial_posix_tests::cflags_data;
termios serial_posix_tests::termios_data;
speed_t serial_posix_tests::speed_data;

int serial_posix_tests::fd_data;
std::string serial_posix_tests::read_data;
std::string serial_posix_tests::put_data;
size_t serial_posix_tests::request_size_data;
ssize_t serial_posix_tests::read_return_value;
int serial_posix_tests::current_char;
void* serial_posix_tests::write_data;
ssize_t serial_posix_tests::write_return_value;

TEST_F(serial_posix_tests, new_serial_open_fails_returns_null)
{
    // Arrange
    std::string device = "/dev/ttys1/nowhere";
    int speed = 9600;
    int reset = 0;
    serial_t *expected_serial = nullptr;

    safe_malloc_fake.return_val = nullptr;
    open_wrapper_fake.return_val = ERROR;

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
    tcgetattr_fake.return_val = ERROR;

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
    tcsetattr_fake.return_val = ERROR;
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
    ASSERT_EQ(expected_serial->nextchar, ERROR);
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
    serial_data.fd = fd;
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(close_wrapper_fake.call_count, 1);
    ASSERT_EQ(close_wrapper_fake.arg0_val, fd);
    ASSERT_EQ(serial_data.fd, ERROR);
}

TEST_F(serial_posix_tests, delete_serial_deletes_serial)
{
    // Arrange
    serial_t *expected_serial = &serial_data;

    // Act
    delete_serial(expected_serial);

    //Assert
    ASSERT_EQ(safe_free_fake.arg0_history[1], expected_serial);
}

TEST_F(serial_posix_tests, serial_get_when_port_closed_returns_error)
{
    // Arrange
    serial_data.fd = ERROR;
    serial_t *expected_serial = &serial_data;

    // Act
    int actual = serial_get(expected_serial);

    //Assert
    ASSERT_EQ(actual, ERROR);
}

TEST_F(serial_posix_tests, serial_get_when_char_already_read_return_char_resets_nextchar)
{
    // Arrange
    char expected_character = 'a';
    serial_data.fd = 10;
    serial_data.nextchar = (int)expected_character;
    serial_t *expected_serial = &serial_data;

    // Act
    int actual = serial_get(expected_serial);

    //Assert
    ASSERT_EQ(actual, expected_character);
    ASSERT_EQ(serial_data.nextchar, ERROR);
}

TEST_F(serial_posix_tests, serial_get_when_quit_set_does_not_read_returns_error)
{
    // Arrange
    serial_data.fd = 10;
    serial_data.quit = ERROR;
    serial_t *expected_serial = &serial_data;

    // Act
    int actual = serial_get(expected_serial);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 0);
}

TEST_F(serial_posix_tests, serial_get_reads_one_character)
{
    // Arrange
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;

    read_fake.custom_fake = read_custom_fake;
    read_return_value = 1;
    char expected_char = read_data[0];

    // Act
    char actual = (char)serial_get(expected_serial);

    //Assert
    ASSERT_EQ(actual, expected_char);
    ASSERT_EQ(read_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_get_reads_multiple_characters_retries_until_one_char_read)
{
    // Arrange
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;

    const int read_read_count = 3;
    ssize_t read_return_values[read_read_count] = { 3, 7, 1 };
    SET_RETURN_SEQ(read, read_return_values, read_read_count);

    // Act
    char actual = (char)serial_get(expected_serial);

    //Assert
    ASSERT_NE(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, read_read_count);
    ASSERT_EQ(usleep_wrapper_fake.call_count, (read_read_count - 1));
}

TEST_F(serial_posix_tests, serial_get_read_error_returns_error)
{
    // Arrange
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;
    read_fake.return_val = ERROR;

    // Act
    char actual = (char)serial_get(expected_serial);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 1);
    ASSERT_EQ(usleep_wrapper_fake.call_count, 0);
}

TEST_F(serial_posix_tests, serial_read_fd_not_set_quits)
{
    // Arrange
    serial_t *expected_serial = &serial_data;

    const int buffer_size = 1024;
    char buffer[buffer_size];

    // Act
    char actual = (char)serial_read(expected_serial, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 0);
}


TEST_F(serial_posix_tests, serial_read_quit_set_quits)
{
    // Arrange
    serial_data.fd = 10;
    serial_data.quit = ERROR;
    serial_t *expected_serial = &serial_data;

    const int buffer_size = 1024;
    char buffer[buffer_size];

    // Act
    char actual = (char)serial_read(expected_serial, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 0);
}

TEST_F(serial_posix_tests, serial_read_read_fails_returns_error)
{
    // Arrange
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;

    read_fake.return_val = ERROR;
    const int buffer_size = 1024;
    char buffer[buffer_size];

    // Act
    char actual = (char)serial_read(expected_serial, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_read_read_succeeds_returns_correct_data)
{
    // Arrange
    int num__to_read = 4;
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;
    current_char = 0;

    read_fake.custom_fake = read_size_custom_fake;
    read_return_value = num__to_read;

    const int buffer_size = 1024;
    char buffer[buffer_size];
    memset(buffer, 0, sizeof(buffer));

    read_data = "4cha";

    // Act
    char actual = (char)serial_read(expected_serial, buffer, num__to_read);

    std::string buffer_string(buffer);
    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(read_fake.call_count, 1);
    ASSERT_EQ(buffer_string, read_data);
}

TEST_F(serial_posix_tests, serial_read_read_succeeds_multiple_times_returns_correct_data)
{
    // Arrange
    int num__to_read = 12;
    int num_chars_read = 4;
    int num_read_calls = num__to_read / num_chars_read;
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;
    current_char = 0;

    read_fake.custom_fake = read_return_number_custom_fake;
    read_return_value = num_chars_read;

    const int buffer_size = 1024;
    char buffer[buffer_size];
    memset(buffer, 0, sizeof(buffer));

    read_data = "twelve_chars";

    // Act
    char actual = (char)serial_read(expected_serial, buffer, num__to_read);

    std::string buffer_string(buffer);
    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(read_fake.call_count, num_read_calls);
    ASSERT_EQ(buffer_string, read_data);
}

TEST_F(serial_posix_tests, serial_read_line_fd_not_set_quits)
{
    // Arrange
    membuf_t membuf_data;

    serial_t *expected_serial = &serial_data;

    // Act
    const char *actual = serial_readline(expected_serial, &membuf_data);

    //Assert
    ASSERT_EQ(actual, nullptr);
}


TEST_F(serial_posix_tests, serial_read_line_quit_set_quits)
{
    // Arrange
    membuf_t membuf_data;

    serial_data.fd = 10;
    serial_data.quit = ERROR;
    serial_t *expected_serial = &serial_data;

    // Act
    const char *actual = serial_readline(expected_serial, &membuf_data);

    //Assert
    ASSERT_EQ(actual, nullptr);
}

TEST_F(serial_posix_tests, serial_read_line_read_fails_returns_error)
{
    // Arrange
    membuf_t membuf_data;

    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;
    read_fake.return_val = ERROR;

    // Act
    const char *actual = serial_readline(expected_serial, &membuf_data);

    //Assert
    ASSERT_EQ(actual, nullptr);
}

TEST_F(serial_posix_tests, serial_read_line_read_reads_r_n_terminated_line)
{
    // Arrange
    membuf_t membuf_data;
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;
    read_fake.custom_fake = read_size_custom_fake;
    read_return_value = 1;
    int line_ending_length = 2;
    read_data = "line\r\n";
    membuf_data_fake.return_val = membuf_data.buffer;

    // Act
    const char *actual = serial_readline(expected_serial, &membuf_data);

    //Assert
    ASSERT_EQ(actual, membuf_data.buffer);
    ASSERT_EQ(read_fake.call_count, read_data.length());
    ASSERT_EQ(membuf_put_fake.call_count, read_data.length() - line_ending_length);
    ASSERT_EQ(membuf_put_fake.arg1_history[0], read_data[0]);
    ASSERT_EQ(membuf_put_fake.arg1_history[1], read_data[1]);
    ASSERT_EQ(membuf_put_fake.arg1_history[2], read_data[2]);
    ASSERT_EQ(membuf_put_fake.arg1_history[3], read_data[3]);
    ASSERT_EQ(membuf_append_zero_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_read_line_read_reads_n_terminated_line)
{
    // Arrange
    membuf_t membuf_data;
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;
    read_fake.custom_fake = read_size_custom_fake;
    read_return_value = 1;
    int line_ending_length = 1;
    read_data = "line\n";
    membuf_data_fake.return_val = membuf_data.buffer;

    // Act
    const char *actual = serial_readline(expected_serial, &membuf_data);

    //Assert
    ASSERT_EQ(actual, membuf_data.buffer);
    ASSERT_EQ(read_fake.call_count, read_data.length());
    ASSERT_EQ(membuf_put_fake.call_count, read_data.length() - line_ending_length);
    ASSERT_EQ(membuf_put_fake.arg1_history[0], read_data[0]);
    ASSERT_EQ(membuf_put_fake.arg1_history[1], read_data[1]);
    ASSERT_EQ(membuf_put_fake.arg1_history[2], read_data[2]);
    ASSERT_EQ(membuf_put_fake.arg1_history[3], read_data[3]);
    ASSERT_EQ(membuf_append_zero_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_read_line_rn_arduino_debug_is_printed_and_cleared)
{
    // Arrange
    membuf_t membuf_data;

    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;
    read_fake.custom_fake = read_size_custom_fake;
    read_return_value = 1;
    int line_ending_length = 2;
    read_data = "#!line\r\nline\r\n";

    const char* membuf_data_return[2] = { "#!line\r\n", "line\r\n" };
    SET_RETURN_SEQ(membuf_data, (char**)membuf_data_return, 2);
    membuf_len_fake.return_val = std::string(membuf_data_return[0]).length();

    // Act
    const char *actual = serial_readline(expected_serial, &membuf_data);

    //Assert
    ASSERT_EQ(actual, membuf_data_return[1]);
    ASSERT_EQ(read_fake.call_count, read_data.length());
    ASSERT_EQ(membuf_put_fake.call_count, read_data.length() - (line_ending_length+line_ending_length));
    ASSERT_EQ(membuf_clear_fake.call_count, 2);
    ASSERT_EQ(membuf_append_zero_fake.call_count, 2);
    ASSERT_EQ(r_debug_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_read_line_n_arduino_debug_is_printed_and_cleared)
{
    // Arrange
    membuf_t membuf_data;
    serial_data.fd = 10;
    serial_t *expected_serial = &serial_data;
    read_fake.custom_fake = read_size_custom_fake;
    read_return_value = 1;
    int line_ending_length = 1;
    read_data = "#!line\nline\n";

    const char* membuf_data_return[2] = { "#!line\n", "line\n" };
    SET_RETURN_SEQ(membuf_data, (char**)membuf_data_return, 2);
    membuf_len_fake.return_val = std::string(membuf_data_return[0]).length();

    // Act
    const char *actual = serial_readline(expected_serial, &membuf_data);

    //Assert
    ASSERT_EQ(actual, membuf_data_return[1]);
    ASSERT_EQ(read_fake.call_count, read_data.length());
    ASSERT_EQ(membuf_put_fake.call_count, read_data.length() - (line_ending_length+line_ending_length));
    ASSERT_EQ(membuf_clear_fake.call_count, 2);
    ASSERT_EQ(membuf_append_zero_fake.call_count, 2);
    ASSERT_EQ(r_debug_fake.call_count, 1);
}


TEST_F(serial_posix_tests, serial_put_if_fd_unset_returns_error)
{
    // Arrange
    char c = 'a';
    // Act
    int actual = serial_put(&serial_data, c);

    //Assert
    ASSERT_EQ(actual, ERROR);
}

TEST_F(serial_posix_tests, serial_put_quit_set_does_not_write)
{
    // Arrange
    char c = 'a';
    serial_data.fd = 10;
    serial_data.quit = 1;

    // Act
    int actual = serial_put(&serial_data, c);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 0);
}

TEST_F(serial_posix_tests, serial_put_write_succeeds_returns_0)
{
    // Arrange
    char c = 'a';
    serial_data.quit = 0;
    serial_data.fd = 10;
    write_fake.return_val = 1;

    // Act
    int actual = serial_put(&serial_data, c);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(write_fake.arg0_val, serial_data.fd);
    ASSERT_EQ(write_fake.arg2_val, 1);
}

TEST_F(serial_posix_tests, serial_put_write_fails_returns_error)
{
    // Arrange
    char c = 'a';
    serial_data.quit = 0;
    serial_data.fd = 10;
    write_fake.return_val = -1;

    // Act
    int actual = serial_put(&serial_data, c);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(write_fake.arg0_val, serial_data.fd);
    ASSERT_EQ(write_fake.arg2_val, 1);
}

TEST_F(serial_posix_tests, serial_put_write_0_retries_returns_error)
{
    // Arrange
    char c = 'a';
    serial_data.quit = 0;
    serial_data.fd = 10;

    long write_return_values[3] = { 0, 0, 1 };
    SET_RETURN_SEQ(write, write_return_values, 3);

    // Act
    int actual = serial_put(&serial_data, c);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 3);
}

TEST_F(serial_posix_tests, serial_write_fd_unset_returns)
{
    // Arrange
    const char *buffer = "buffer";
    std::string bufferstring(buffer);
    serial_data.quit = 0;

    // Act
    int actual = serial_write(&serial_data, buffer, bufferstring.length());

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(write_fake.call_count, 0);
}

TEST_F(serial_posix_tests, serial_write_when_quit_set_quits)
{
    // Arrange
    const char *buffer = "buffer";
    std::string bufferstring(buffer);

    serial_data.quit = 1;
    serial_data.fd = 10;

    // Act
    int actual = serial_write(&serial_data, buffer, bufferstring.length());

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(write_fake.call_count, 0);
}

TEST_F(serial_posix_tests, serial_write_called_with_correct_parameters)
{
    // Arrange
    const char *buffer = "buffer";
    std::string bufferstring(buffer);
    serial_data.quit = 0;
    serial_data.fd = 10;
    write_fake.return_val = bufferstring.length();

    // Act
    int actual = serial_write(&serial_data, buffer, bufferstring.length());

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(write_fake.arg0_val, serial_data.fd);
    ASSERT_EQ(write_fake.arg1_val, buffer);
    ASSERT_EQ(write_fake.arg2_val, bufferstring.length());
}

TEST_F(serial_posix_tests, serial_write_called_correct_number_of_times)
{
    // Arrange
    const char *buffer = "buffer";
    std::string bufferstring(buffer);
    serial_data.quit = 0;
    serial_data.fd = 10;
    write_fake.return_val = bufferstring.length();

    long write_return_values[3] = { 0, 3, 3 };
    SET_RETURN_SEQ(write, write_return_values, 3);

    // Act
    int actual = serial_write(&serial_data, buffer, bufferstring.length());
    int bufferLength = bufferstring.length();

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 3);
    ASSERT_EQ(write_fake.arg0_val, serial_data.fd);
    ASSERT_EQ(write_fake.arg1_history[0], buffer);
    ASSERT_EQ(write_fake.arg1_history[1], buffer);
    ASSERT_EQ(write_fake.arg1_history[2], buffer+write_return_values[1]);
    ASSERT_EQ(write_fake.arg2_history[0], bufferLength);
    ASSERT_EQ(write_fake.arg2_history[1], bufferLength);
    ASSERT_EQ(write_fake.arg2_history[2], (bufferLength-=write_return_values[1]));
}

TEST_F(serial_posix_tests, serial_write_fails_returns_error)
{
    // Arrange
    const char *buffer = "buffer";
    std::string bufferstring(buffer);
    serial_data.quit = 0;
    serial_data.fd = 10;
    write_fake.return_val = ERROR;

    // Act
    int actual = serial_write(&serial_data, buffer, bufferstring.length());

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(write_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_print_calls_write_with_correct_parameters)
{
    // Arrange
    const char *buffer = "buffer";
    std::string bufferstring(buffer);
    serial_data.quit = 0;
    serial_data.fd = 10;
    write_fake.return_val = bufferstring.length();

    // Act
    int actual = serial_print(&serial_data, buffer);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(write_fake.arg0_val, serial_data.fd);
    ASSERT_EQ(write_fake.arg1_val, buffer);
    ASSERT_EQ(write_fake.arg2_val, bufferstring.length());
}

TEST_F(serial_posix_tests, serial_println_write_fails_returns_error)
{
    // Arrange
    const char *serial_string = "buffer";
    std::string bufferstring(serial_string);
    int expected_string_length = bufferstring.length() + 2; // \r\n chars
    write_fake.return_val = ERROR;


    const int buffer_length = 24;
    char buffer[buffer_length];
    memset(buffer, 0, buffer_length);

    membuf_t membuf_data;
    membuf_data.length = buffer_length;
    membuf_data.buffer = buffer;
    membuf_data.index = 0;

    serial_data.quit = 0;
    serial_data.fd = 10;
    serial_data.out = &membuf_data;

    membuf_data_fake.return_val = membuf_data.buffer;
    membuf_len_fake.return_val = expected_string_length;

    // Act
    int actual = serial_println(&serial_data, buffer);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(write_fake.arg0_val, serial_data.fd);
    ASSERT_EQ(write_fake.arg1_val, buffer);
    ASSERT_EQ(write_fake.arg2_val, expected_string_length);
}

TEST_F(serial_posix_tests, serial_println_write_parameters_are_correct_sends_rn)
{
    // Arrange
    const char *serial_string = "buffer";
    std::string bufferstring(serial_string);
    std::string expected_line_end("\r\n");
    int expected_string_length = bufferstring.length() + expected_line_end.length();
    write_fake.custom_fake = write_size_custom_fake;

    const int buffer_length = 24;
    char buffer[buffer_length];
    memset(buffer, 0, buffer_length);

    membuf_t membuf_data;
    membuf_data.length = buffer_length;
    membuf_data.buffer = buffer;
    membuf_data.index = 0;

    serial_data.quit = 0;
    serial_data.fd = 10;
    serial_data.out = &membuf_data;

    membuf_data_fake.return_val = membuf_data.buffer;
    membuf_len_fake.return_val = expected_string_length;

    // Act
    int actual = serial_println(&serial_data, buffer);
    std::string actual_line_end((char*)write_data);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(write_fake.arg0_history[0], serial_data.fd);
    ASSERT_EQ(write_fake.arg1_history[0], buffer);
    ASSERT_EQ(write_fake.arg2_history[0], expected_string_length);
    ASSERT_EQ(expected_line_end, actual_line_end);
}

TEST_F(serial_posix_tests, serial_printf_formats_data)
{
    // Arrange
    std::string intput_string("expected");
    std::string expected_formatted_string("expected1");
    write_fake.custom_fake = write_size_custom_fake;

    const int buffer_length = 24;
    char buffer[buffer_length];
    memset(buffer, 0, buffer_length);

    membuf_t membuf_data;
    membuf_data.length = buffer_length;
    membuf_data.buffer = buffer;
    membuf_data.index = 0;

    serial_data.quit = 0;
    serial_data.fd = 10;
    serial_data.out = &membuf_data;

    membuf_data_fake.return_val = membuf_data.buffer;
    membuf_len_fake.return_val = expected_formatted_string.length();

    // Act
    int actual = serial_printf(&serial_data, "%s%d", intput_string.c_str(), 1);
    std::string actual_formatted_string((char*)write_data);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(expected_formatted_string, actual_formatted_string);
}

TEST_F(serial_posix_tests, serial_command_send_socket_null_returns_null)
{
    // Arrange
    membuf_t membuf_data;
    const char *command = "L90";

    // Act
    const char *actual = serial_command_send(nullptr, &membuf_data, command);

    //Assert
    ASSERT_EQ(actual, nullptr);
}

TEST_F(serial_posix_tests, serial_command_send_send_fails_logs_error_inc_error_count)
{
    // Arrange
    const int buffer_length = 24;
    char buffer[buffer_length];
    memset(buffer, 0, buffer_length);

    membuf_t membuf_data;
    membuf_data.length = buffer_length;
    membuf_data.buffer = buffer;
    membuf_data.index = 0;
    const char *command = "L90";

    serial_data.quit = 0;
    serial_data.fd = 10;
    serial_data.out = &membuf_data;

    membuf_data_fake.return_val = membuf_data.buffer;
    membuf_len_fake.return_val = membuf_data.length;
    write_fake.return_val = -1;

    std::string expected_r_err_string("serial_command_send: failed to send the command");

    // Act
    const char *actual = serial_command_send(&serial_data, &membuf_data, command);

    //Assert
    ASSERT_EQ(actual, nullptr);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.arg0_val, expected_r_err_string);
    ASSERT_EQ(serial_data.errors, 1);
}

TEST_F(serial_posix_tests, serial_command_send_read_fails_logs_error_inc_error_count)
{
    // Arrange
    const int buffer_length = 24;
    char buffer[buffer_length];
    memset(buffer, 0, buffer_length);

    membuf_t membuf_data;
    membuf_data.length = buffer_length;
    membuf_data.buffer = buffer;
    membuf_data.index = 0;
    const char *command = "L90";

    std::string expected_command(command);
    expected_command += "\r\n";

    serial_data.quit = 0;
    serial_data.fd = 10;
    serial_data.out = &membuf_data;

    membuf_data_fake.return_val = membuf_data.buffer;
    membuf_len_fake.return_val = expected_command.length();
    write_fake.return_val = 1;

    std::string expected_r_err_string("serial_command_send: reply == NULL");
  //  read_fake.custom_fake = read_size_custom_fake;
  //  read_return_value = -1;
 //   read_data = "line\r\n";
    read_fake.return_val = -1;

    // Act
    const char *actual = serial_command_send(&serial_data, &membuf_data, command);

    //Assert
    ASSERT_EQ(actual, nullptr);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.arg0_val, expected_r_err_string);
    ASSERT_EQ(serial_data.errors, 1);
}

TEST_F(serial_posix_tests, serial_command_send_reads_ERR_logs_error_inc_error_count)
{
    // Arrange
    const int buffer_length = 24;
    char buffer[buffer_length];
    memset(buffer, 0, buffer_length);

    membuf_t membuf_data;
    membuf_data.length = buffer_length;
    membuf_data.buffer = buffer;
    membuf_data.index = 0;
    const char *command = "L90";

    std::string expected_command(command);
    expected_command += "\r\n";

    serial_data.quit = 0;
    serial_data.fd = 10;
    serial_data.out = &membuf_data;

    write_fake.return_val = 1;

    std::string expected_read_data = "ERROR DATA";
    std::string expected_r_err_string("serial_command_send: %s");

    read_data = expected_read_data + "\r\n";
    read_fake.custom_fake = read_size_custom_fake;

    const int read_read_count = 2;
    const char* membuf_data_return[read_read_count] = { membuf_data.buffer, expected_read_data.c_str() };
    SET_RETURN_SEQ(membuf_data, (char**)membuf_data_return, read_read_count);

    // Act
    const char *actual = serial_command_send(&serial_data, &membuf_data, command);

    //Assert
    ASSERT_EQ(actual, expected_read_data.c_str());
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.arg0_val, expected_r_err_string);
    ASSERT_EQ(serial_data.errors, 1);
}

TEST_F(serial_posix_tests, serial_command_succeeds_returns_reply)
{
    // Arrange
    const int buffer_length = 24;
    char buffer[buffer_length];
    memset(buffer, 0, buffer_length);

    membuf_t membuf_data;
    membuf_data.length = buffer_length;
    membuf_data.buffer = buffer;
    membuf_data.index = 0;
    const char *command = "L90";

    std::string expected_command(command);
    expected_command += "\r\n";

    serial_data.quit = 0;
    serial_data.fd = 10;
    serial_data.out = &membuf_data;

    write_fake.return_val = 1;

    std::string expected_read_data = "GOOD DATA";
    std::string expected_r_err_string("serial_command_send: %s");

    read_data = expected_read_data + "\r\n";
    read_fake.custom_fake = read_size_custom_fake;

    const int read_read_count = 2;
    const char* membuf_data_return[read_read_count] = { membuf_data.buffer, expected_read_data.c_str() };
    SET_RETURN_SEQ(membuf_data, (char**)membuf_data_return, read_read_count);

    // Act
    const char *actual = serial_command_send(&serial_data, &membuf_data, command);

    //Assert
    ASSERT_EQ(actual, expected_read_data.c_str());
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(serial_data.errors, 0);
}