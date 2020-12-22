#ifndef ROMI_ROVER_BUILD_AND_TEST_ILINUX_H
#define ROMI_ROVER_BUILD_AND_TEST_ILINUX_H

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#include <stdarg.h>
#include <signal.h>
#include <dirent.h>
#include <stdio.h>

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

namespace rpp {

    class ILinux {
    public:
        virtual ~ILinux() {};

        virtual int open(const char *pathname, int flags) = 0;

        virtual int close(int fd) = 0;

        virtual void exit(int status) = 0;

        virtual sighandler_t signal(int signum, sighandler_t handler) = 0;

        virtual int stat(const char *path, struct stat *buf) = 0;

        virtual pid_t waitpid(pid_t pid, int *status, int options) = 0;

        virtual int execve(const char *filename, char *const argv[], char *const envp[]) = 0;

        virtual pid_t fork(void) = 0;

        virtual int kill(pid_t pid, int sig) = 0;

        virtual int system(const char *command) = 0;

        virtual FILE *fopen(const char *filename, const char *mode) = 0;

        virtual int fclose(FILE *fp) = 0;

        virtual DIR *opendir(const char *directory) = 0;

        virtual int closedir(DIR *) = 0;

        virtual struct dirent *readdir(DIR *directory) = 0;

        virtual int remove(const char *s) = 0;

        virtual unsigned int sleep(unsigned int seconds) = 0;

        virtual int ioctl(int fd, unsigned long request, void *argp) = 0;
        
        virtual int poll(struct pollfd *fds, nfds_t nfds, int timeout) = 0;

        virtual ssize_t read(int fd, void *buf, size_t count) = 0;
    };

}
#endif //ROMI_ROVER_BUILD_AND_TEST_ILINUX_H
