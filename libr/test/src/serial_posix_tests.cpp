#include <fcntl.h>
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
        int fd = 10;
        RESET_FAKE(new_mutex);
        RESET_FAKE(delete_mutex);
        RESET_FAKE(mutex_lock);
        RESET_FAKE(mutex_unlock);

        RESET_FAKE(tcgetattr)
        RESET_FAKE(cfsetspeed)
        RESET_FAKE(tcflush)
        RESET_FAKE(tcsetattr)

        RESET_FAKE(r_err);
        RESET_FAKE(r_debug);

        RESET_FAKE(open_wrapper)
        RESET_FAKE(close_wrapper)
        RESET_FAKE(usleep_wrapper)

        RESET_FAKE(read)
        RESET_FAKE(write)
        actual_serial = nullptr;
        membuf_data = nullptr;

        read_data = "serial line\n";
        write_data = "";
        read_return_value = 0;
        current_read_char = 0;

        device = "/dev/ttys1";
        port_speed = 9600;
        reset_flag = 0;
        put_data = "";

        open_wrapper_fake.return_val = fd;
        membuf_data = new_membuf();
    }

    void TearDown() override
    {
        if (actual_serial != nullptr)
        {
            delete_serial(actual_serial);
            actual_serial = nullptr;
        }
        if (membuf_data != nullptr)
        {
            delete_membuf(membuf_data);
            membuf_data = nullptr;
        }
    }

    static int tcgetattr_custom_fake(int fd __attribute__((unused)), struct termios *pterm)
    {
        pterm->c_cflag = cflags_data;
        return 0;
    }

    static int tcsetattr_custom_fake(int fd __attribute__((unused)) ,int actions __attribute__((unused)), const struct termios *pterm)
    {
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
        *((char*)data) = read_data[current_read_char++];
        return read_return_value;
    }

    static ssize_t read_size_custom_fake( int fd, void *data, size_t size)
    {
        char* char_data = ((char*)data);
        size_t char_data_index = 0;
        fd_data = fd;
        request_size_data = size;

        while (char_data_index < size)
            char_data[char_data_index++] = read_data[current_read_char++];
        return size;
    }

    static ssize_t read_return_number_custom_fake( int fd, void *data, size_t size)
    {
        char* char_data = ((char*)data);
        size_t char_data_index = 0;
        fd_data = fd;
        request_size_data = size;

        while (char_data_index < (size_t)read_return_value)
            char_data[char_data_index++] = read_data[current_read_char++];
        return read_return_value;
    }

    static ssize_t write_size_custom_fake(int fd, const void *data, size_t size)
    {
        fd_data = fd;
        request_size_data = size;
        write_data += (char *)data;
        return size;
    }

public:
    // Serial Setup data;
    std::string device;
    int port_speed = 0;
    int reset_flag;
    serial_t *actual_serial;
    membuf_t *membuf_data;

    static unsigned int cflags_data;
    static termios termios_data;
    static speed_t speed_data;

    static int fd_data;
    static std::string read_data;
    static std::string put_data;
    static size_t request_size_data;
    static ssize_t read_return_value;
    static int current_read_char;

    static std::string write_data;
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
int serial_posix_tests::current_read_char;
std::string serial_posix_tests::write_data;
ssize_t serial_posix_tests::write_return_value;


TEST_F(serial_posix_tests, new_serial_open_calls_with_correct_parameters)
{
    // Arrange
    serial_t *expected_serial = nullptr;
    open_wrapper_fake.return_val = ERROR;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(open_wrapper_fake.call_count, 1);
    ASSERT_EQ(std::string(open_wrapper_fake.arg0_val), this->device);
    ASSERT_EQ(open_wrapper_fake.arg1_val, (O_RDWR | O_NOCTTY | O_SYNC));
    ASSERT_EQ(r_err_fake.call_count, 1);
}

TEST_F(serial_posix_tests, new_serial_open_fails_returns_null)
{
    // Arrange
    serial_t *expected_serial = nullptr;

 //   safe_malloc_fake.return_val = nullptr;
    open_wrapper_fake.return_val = ERROR;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(open_wrapper_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 1);
}

TEST_F(serial_posix_tests, new_serial_unknown_speed_returns_null)
{
    // Arrange
    port_speed = 1024;

    serial_t *expected_serial = nullptr;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(r_err_fake.call_count, 1);
}

TEST_F(serial_posix_tests, new_serial_getattr_fails_closes_port_returns_null)
{
    // Arrange
    serial_t *expected_serial = nullptr;

//    int fd = 1;
//    open_wrapper_fake.return_val = fd;
    tcgetattr_fake.return_val = ERROR;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

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
    port_speed = 9600;
    serial_t *expected_serial = nullptr;

    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = ERROR;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B9600);
}

TEST_F(serial_posix_tests, new_serial_19200_speed_is_set_correctly)
{
    // Arrange
    port_speed = 19200;
    serial_t *expected_serial = nullptr;

    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = ERROR;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B19200);
}


TEST_F(serial_posix_tests, new_serial_38400_speed_is_set_correctly)
{
    // Arrange
    std::string device = "/dev/ttys1";
    port_speed = 38400;
    serial_t *expected_serial = nullptr;

    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = ERROR;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B38400);
}

TEST_F(serial_posix_tests, new_serial_57600_speed_is_set_correctly)
{
    // Arrange
    port_speed = 57600;
    serial_t *expected_serial = nullptr;

    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = ERROR;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B57600);
}


TEST_F(serial_posix_tests, new_serial_115200_speed_is_set_correctly)
{
    // Arrange
    port_speed = 115200;
    serial_t *expected_serial = nullptr;

    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = ERROR;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial, expected_serial);
    ASSERT_EQ(cfsetspeed_fake.arg1_val, B115200);
}


TEST_F(serial_posix_tests, new_serial_if_reset_HUPCL_flag_cleared)
{
    // Arrange
    port_speed = 115200;

    tcgetattr_fake.return_val = 0;
    cfsetspeed_fake.custom_fake = cfsetspeed_custom_fake;
    tcsetattr_fake.custom_fake = tcsetattr_custom_fake;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);
    unsigned int HUPCL_flag = termios_data.c_cflag & HUPCL;

    //Assert
    ASSERT_EQ(HUPCL_flag, 0);
}


///// NOT WORKING
TEST_F(serial_posix_tests, new_serial_if_reset_clear_HUPCL_flag_set)
{
    // Arrange
//    std::string device = "/dev/ttys1";
    port_speed = 115200;
    reset_flag = 1;

    tcsetattr_fake.custom_fake = tcsetattr_custom_fake;
    tcgetattr_fake.custom_fake = tcgetattr_custom_fake;
    cfsetspeed_fake.custom_fake = cfsetspeed_custom_fake;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);
    unsigned int HUPCL_flag = termios_data.c_cflag & HUPCL;

    //Assert
    ASSERT_EQ(HUPCL_flag, HUPCL);
}

TEST_F(serial_posix_tests, new_serial_port_is_flushed)
{
    // Arrange
    int fd = 10;
    open_wrapper_fake.return_val = fd;
    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = 0;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_NE(actual_serial, nullptr);
    ASSERT_EQ(tcflush_fake.call_count, 1);
    ASSERT_EQ(tcflush_fake.arg0_val, fd);
    ASSERT_EQ(tcflush_fake.arg1_val, TCIOFLUSH);
}


TEST_F(serial_posix_tests, new_serial_setattr_sets_correct_attributes)
{
    // Arrange
    int fd = 10;
    open_wrapper_fake.return_val = fd;
    tcgetattr_fake.custom_fake = tcgetattr_custom_fake;
    tcsetattr_fake.custom_fake = tcsetattr_custom_fake;
    cflags_data = 0;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_NE(actual_serial, nullptr);
    ASSERT_EQ(tcsetattr_fake.arg0_val, fd);
    ASSERT_EQ(tcsetattr_fake.arg1_val, TCSANOW);
    ASSERT_EQ(termios_data.c_cflag, (CLOCAL | CREAD | CS8) );
    ASSERT_EQ(termios_data.c_lflag, (ICANON | ISIG) );
    ASSERT_EQ(termios_data.c_iflag, (IGNCR) );
    ASSERT_EQ(termios_data.c_oflag, (0) );
}


TEST_F(serial_posix_tests, new_serial_valid_data_sets_data_returns_serial)
{
    // Arrange
    int fd = 1;
    open_wrapper_fake.return_val = fd;
    tcgetattr_fake.return_val = 0;
    tcsetattr_fake.return_val = 0;

    mutex_t mutex_data;
    new_mutex_fake.return_val = &mutex_data;

    // Act
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    //Assert
    ASSERT_EQ(actual_serial->device, device);
    ASSERT_EQ(actual_serial->mutex, &mutex_data);
    ASSERT_EQ(actual_serial->errors, 0);
    ASSERT_EQ(actual_serial->fd, fd);
    ASSERT_EQ(actual_serial->nextchar, ERROR);
    ASSERT_EQ(actual_serial->speed, port_speed);
    ASSERT_EQ(actual_serial->quit, 0);
}

TEST_F(serial_posix_tests, delete_serial_when_NULL_does_not_delete)
{
    // Arrange
    // Act
    delete_serial(nullptr);

    //Assert
    ASSERT_EQ(close_wrapper_fake.call_count, 0);
}


TEST_F(serial_posix_tests, delete_serial_locks_and_unlocks_mutex)
{
    // Arrange
    mutex_t mutex_data;
    new_mutex_fake.return_val = &mutex_data;
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    // Act
    delete_serial(actual_serial);
    actual_serial = nullptr;

    //Assert
    ASSERT_EQ(mutex_lock_fake.call_count, 1);
    ASSERT_EQ(mutex_lock_fake.arg0_val, &mutex_data);
    ASSERT_EQ(mutex_unlock_fake.call_count, 1);
    ASSERT_EQ(mutex_unlock_fake.arg0_val, &mutex_data);
}


TEST_F(serial_posix_tests, delete_serial_deletes_mutex)
{
    // Arrange
    mutex_t mutex_data;
    new_mutex_fake.return_val = &mutex_data;

    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    // Act
    delete_serial(actual_serial);
    actual_serial = nullptr;

    //Assert
    ASSERT_EQ(delete_mutex_fake.call_count, 2); // One for Membuff
    ASSERT_EQ(delete_mutex_fake.arg0_history[0], &mutex_data);
    ASSERT_EQ(delete_mutex_fake.arg0_history[1], &mutex_data);
}

TEST_F(serial_posix_tests, delete_serial_closes_fd)
{
    // Arrange
    int fd = 10;
    open_wrapper_fake.return_val = fd;
    actual_serial =  new_serial(device.c_str(), port_speed, reset_flag);

    // Act
    delete_serial(actual_serial);
    actual_serial = nullptr;

    //Assert
    ASSERT_EQ(close_wrapper_fake.call_count, 1);
    ASSERT_EQ(close_wrapper_fake.arg0_val, fd);
}

TEST_F(serial_posix_tests, serial_get_when_port_closed_returns_error)
{
    // Arrange
    serial_t serial_data;
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
    serial_t serial_data;
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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = ERROR;
    serial_data.nextchar = ERROR;
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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    const int read_read_count = 3;
    ssize_t read_return_values[read_read_count] = { 3, 7, 1 };
    SET_RETURN_SEQ(read, read_return_values, read_read_count);

    // Act
    int actual = serial_get(expected_serial);

    //Assert
    ASSERT_NE(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, read_read_count);
    ASSERT_EQ(usleep_wrapper_fake.call_count, (read_read_count - 1));
}

TEST_F(serial_posix_tests, serial_get_read_error_returns_error)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;
    read_fake.return_val = ERROR;

    // Act
    int actual = serial_get(expected_serial);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 1);
    ASSERT_EQ(usleep_wrapper_fake.call_count, 0);
}


TEST_F(serial_posix_tests, serial_read_fd_not_set_quits)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = -1;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    const int buffer_size = 1024;
    char buffer[buffer_size];

    // Act
    int actual = serial_read(expected_serial, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 0);
}


TEST_F(serial_posix_tests, serial_read_quit_set_quits)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = ERROR;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    const int buffer_size = 1024;
    char buffer[buffer_size];

    // Act
    int actual = serial_read(expected_serial, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 0);
}

TEST_F(serial_posix_tests, serial_read_read_fails_returns_error)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    read_fake.return_val = ERROR;
    const int buffer_size = 1024;
    char buffer[buffer_size];

    // Act
    int actual = serial_read(expected_serial, buffer, buffer_size);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(read_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_read_read_succeeds_returns_correct_data)
{
    // Arrange
    int num__to_read = 4;
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;
    
    current_read_char = 0;
    read_fake.custom_fake = read_size_custom_fake;
    read_return_value = num__to_read;

    const int buffer_size = 1024;
    char buffer[buffer_size];
    memset(buffer, 0, sizeof(buffer));

    read_data = "4cha";

    // Act
    int actual = serial_read(expected_serial, buffer, num__to_read);

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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;
    current_read_char = 0;

    read_fake.custom_fake = read_return_number_custom_fake;
    read_return_value = num_chars_read;

    const int buffer_size = 1024;
    char buffer[buffer_size];
    memset(buffer, 0, sizeof(buffer));

    read_data = "twelve_chars";

    // Act
    int actual = serial_read(expected_serial, buffer, num__to_read);

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

    serial_t serial_data;
    serial_data.fd = ERROR;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    // Act
    const char *actual = serial_readline(expected_serial, &membuf_data);

    //Assert
    ASSERT_EQ(actual, nullptr);
}


TEST_F(serial_posix_tests, serial_read_line_quit_set_quits)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = ERROR;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    // Act
    const char *actual = serial_readline(expected_serial, membuf_data);

    //Assert
    ASSERT_EQ(actual, nullptr);
}

TEST_F(serial_posix_tests, serial_read_line_read_fails_returns_error)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;
    read_fake.return_val = ERROR;

    // Act
    const char *actual = serial_readline(expected_serial, membuf_data);

    //Assert
    ASSERT_EQ(actual, nullptr);
}

TEST_F(serial_posix_tests, serial_read_line_read_reads_r_n_terminated_line)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    read_fake.custom_fake = read_size_custom_fake;
    std::string expected_string = "line";
    read_data = expected_string + "\r\n";

    // Act
    const char *actual = serial_readline(expected_serial, membuf_data);

    //Assert
    ASSERT_EQ(read_fake.call_count, read_data.length());
    ASSERT_EQ(actual, membuf_data->buffer);
    ASSERT_EQ(std::string(membuf_data->buffer), expected_string);
    ASSERT_EQ(std::string(actual), expected_string);
}

TEST_F(serial_posix_tests, serial_read_line_read_reads_n_terminated_line)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    read_fake.custom_fake = read_size_custom_fake;
    std::string expected_string = "line";
    read_data = expected_string + "\n";

    // Act
    const char *actual = serial_readline(expected_serial, membuf_data);

    //Assert
    ASSERT_EQ(std::string(actual), expected_string);
    ASSERT_EQ(std::string(membuf_data->buffer), expected_string);
    ASSERT_EQ(read_fake.call_count, read_data.length());
}


TEST_F(serial_posix_tests, serial_read_line_rn_arduino_debug_is_printed_and_cleared)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    read_fake.custom_fake = read_size_custom_fake;
    read_data = "#!line\r\nline\r\n";

    std::string expected_string = "line";

    // Act
    const char *actual = serial_readline(expected_serial, membuf_data);

    //Assert
    ASSERT_EQ(std::string(actual), expected_string);
    ASSERT_EQ(read_fake.call_count, read_data.length());
    ASSERT_EQ(r_debug_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_read_line_n_arduino_debug_is_printed_and_cleared)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_t *expected_serial = &serial_data;

    read_fake.custom_fake = read_size_custom_fake;
    read_data = "#!line\nline\n";
    std::string expected_string = "line";

    // Act
    const char *actual = serial_readline(expected_serial, membuf_data);

    //Assert
    ASSERT_EQ(std::string(actual), expected_string);
    ASSERT_EQ(read_fake.call_count, read_data.length());
    ASSERT_EQ(r_debug_fake.call_count, 1);
}

TEST_F(serial_posix_tests, serial_put_if_fd_unset_returns_error)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = ERROR;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;

    char c = 'a';
    // Act
    int actual = serial_put(&serial_data, c);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(write_fake.call_count, 0);
}

TEST_F(serial_posix_tests, serial_put_quit_set_does_not_write)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 1;
    serial_data.nextchar = ERROR;
    char c = 'a';

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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;

    ssize_t write_return_values[3] = { 0, 0, 1 };
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
    serial_t serial_data;
    serial_data.fd = ERROR;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;

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

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 1;
    serial_data.nextchar = ERROR;

    // Act
    int actual = serial_write(&serial_data, buffer, bufferstring.length());

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(write_fake.call_count, 0);
}


TEST_F(serial_posix_tests, serial_write_called_with_correct_parameters)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;

    const char *buffer = "buffer";
    std::string bufferstring(buffer);
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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;

    const char *buffer = "buffer";
    std::string bufferstring(buffer);
    write_fake.return_val = bufferstring.length();

    ssize_t write_return_values[3] = { 0, 3, 3 };
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
    ASSERT_EQ(write_fake.arg2_history[2], (bufferLength-write_return_values[1]));
}

TEST_F(serial_posix_tests, serial_write_fails_returns_error)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;

    const char *buffer = "buffer";
    std::string bufferstring(buffer);
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
    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;

    const char *buffer = "buffer";
    std::string bufferstring(buffer);
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
    write_fake.return_val = ERROR;

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;

    // Act
    int actual = serial_println(&serial_data, serial_string);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(write_fake.arg0_val, serial_data.fd);
    ASSERT_EQ(write_fake.arg1_val, serial_string);
    ASSERT_EQ(write_fake.arg2_val, bufferstring.length());
}

TEST_F(serial_posix_tests, serial_println_write_parameters_are_correct_sends_rn)
{
    // Arrange
    const char *serial_string = "buffer";
    std::string bufferstring(serial_string);
    std::string expected_line_end("\r\n");
    std::string expected_write_string = bufferstring + expected_line_end;
    write_fake.custom_fake = write_size_custom_fake;

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_data.out = membuf_data;

    // Act
    int actual = serial_println(&serial_data, serial_string);
    std::string actual_line(write_data);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 2);
    ASSERT_EQ(write_fake.arg0_history[0], serial_data.fd);
    ASSERT_EQ(write_fake.arg1_history[0], serial_string);
    ASSERT_EQ(write_fake.arg2_history[0], bufferstring.length());
    ASSERT_EQ(actual_line, expected_write_string);
}

TEST_F(serial_posix_tests, serial_printf_formats_data)
{
    // Arrange
    std::string intput_string("expected");
    std::string expected_formatted_string("expected1");
    write_fake.custom_fake = write_size_custom_fake;

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_data.out = membuf_data;

    // Act
    int actual = serial_printf(&serial_data, "%s%d", intput_string.c_str(), 1);
    std::string actual_formatted_string(write_data);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(write_fake.call_count, 1);
    ASSERT_EQ(expected_formatted_string, actual_formatted_string);
}

TEST_F(serial_posix_tests, serial_command_send_socket_null_returns_null)
{
    // Arrange
    const char *command = "L90";

    // Act
    const char *actual = serial_command_send(nullptr, membuf_data, command);

    //Assert
    ASSERT_EQ(actual, nullptr);
}

TEST_F(serial_posix_tests, serial_command_send_send_fails_logs_error_inc_error_count)
{
    // Arrange
    const char *command = "L90";

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_data.out = membuf_data;
    serial_data.errors = 0;

    write_fake.return_val = -1;

    std::string expected_r_err_string("serial_command_send: failed to send the command");

    // Act
    const char *actual = serial_command_send(&serial_data, membuf_data, command);

    //Assert
    ASSERT_EQ(actual, nullptr);
    ASSERT_EQ(r_err_fake.call_count, 2);
    ASSERT_EQ(r_err_fake.arg0_history[1], expected_r_err_string);
    ASSERT_EQ(serial_data.errors, 1);
}

TEST_F(serial_posix_tests, serial_command_send_read_fails_logs_error_inc_error_count)
{
    // Arrange
    const char *command = "L90";

    std::string expected_command(command);
    expected_command += "\r\n";

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_data.out = membuf_data;
    serial_data.errors = 0;

    write_fake.return_val = 1;

    std::string expected_r_err_string("serial_command_send: reply == NULL");
    read_fake.return_val = -1;

    // Act
    const char *actual = serial_command_send(&serial_data, membuf_data, command);

    //Assert
    ASSERT_EQ(actual, nullptr);
    ASSERT_EQ(r_err_fake.call_count, 2);
    ASSERT_EQ(r_err_fake.arg0_history[1], expected_r_err_string);
    ASSERT_EQ(serial_data.errors, 1);
}


TEST_F(serial_posix_tests, serial_command_send_reads_ERR_logs_error_inc_error_count)
{
    // Arrange
    const char *command = "L90";

    std::string expected_command(command);
    expected_command += "\r\n";

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_data.out = membuf_data;
    serial_data.errors = 0;

    write_fake.return_val = 1;

    std::string expected_read_data = "ERROR DATA";
    std::string expected_r_err_string("serial_command_send: %s");

    read_data = expected_read_data + "\r\n";
    read_fake.custom_fake = read_size_custom_fake;

    // Act
    const char *actual = serial_command_send(&serial_data, membuf_data, command);

    //Assert
    ASSERT_EQ(std::string(actual), expected_read_data);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.arg0_val, expected_r_err_string);
    ASSERT_EQ(serial_data.errors, 1);
}


TEST_F(serial_posix_tests, serial_command_succeeds_returns_reply)
{
    // Arrange
    const char *command = "L90";

    std::string expected_command(command);
    expected_command += "\r\n";

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_data.out = membuf_data;
    serial_data.errors = 0;

    write_fake.return_val = 1;

    std::string expected_read_data = "GOOD DATA";
    std::string expected_r_err_string("serial_command_send: %s");

    read_data = expected_read_data + "\r\n";
    read_fake.custom_fake = read_size_custom_fake;

    // Act
    const char *actual = serial_command_send(&serial_data, membuf_data, command);

    //Assert
    ASSERT_EQ(std::string(actual), expected_read_data);
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(serial_data.errors, 0);
}


TEST_F(serial_posix_tests, serial_command_sendf_socket_null_returns_error)
{
    // Arrange
    const char *command = "L90";

    // Act
    const char *actual = serial_command_sendf(nullptr, membuf_data, "%s", command);

    //Assert
    ASSERT_EQ(actual, nullptr);
}

TEST_F(serial_posix_tests, serial_command_sendf_formats_command)
{
    // Arrange
    std::string command("L90");
    int command_parameter = 111;
    std::string format("%s%d");

    std::string expected_command(command);
    expected_command += std::to_string(command_parameter) + "\r\n";

    serial_t serial_data;
    serial_data.fd = 10;
    serial_data.quit = 0;
    serial_data.nextchar = ERROR;
    serial_data.out = membuf_data;
    serial_data.errors = 0;

    write_fake.custom_fake = write_size_custom_fake;

    std::string expected_read_data = "GOOD DATA";
    std::string expected_r_err_string("serial_command_send: %s");

    read_data = expected_read_data + "\r\n";
    read_fake.custom_fake = read_size_custom_fake;

    // Act
    const char *actual = serial_command_sendf(&serial_data, membuf_data, format.c_str(), command.c_str(), command_parameter);
    std::string actual_command(write_data);

    //Assert
    ASSERT_EQ(std::string(actual), expected_read_data);
    ASSERT_EQ(actual_command, expected_command);
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(serial_data.errors, 0);
}

TEST_F(serial_posix_tests, serial_flush_calls_flush_with_correct_data)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    tcflush_fake.return_val = 0;

    // Act
    int actual = serial_flush(&serial_data);

    //Assert
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(tcflush_fake.arg0_val, serial_data.fd);
    ASSERT_EQ(tcflush_fake.arg1_val, TCIOFLUSH);
}

TEST_F(serial_posix_tests, serial_flush_fails_logs_returns_error)
{
    // Arrange
    serial_t serial_data;
    serial_data.fd = 10;
    tcflush_fake.return_val = ERROR;

    // Act
    int actual = serial_flush(&serial_data);

    //Assert
    ASSERT_EQ(actual, ERROR);
    ASSERT_EQ(r_err_fake.call_count, 1);
}
