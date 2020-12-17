#ifndef ROMI_ROVER_BUILD_AND_TEST_LINUX_H
#define ROMI_ROVER_BUILD_AND_TEST_LINUX_H

#include "ILinux.h"


namespace  rpp {

    class Linux : public ILinux {
    public:
        Linux();

        virtual ~Linux() = default;

        int open(const char *pathname, int flags) override;

        int close(int fd) override;

        void exit(int status) override;

        sighandler_t signal(int signum, sighandler_t handler) override;

        int stat(const char *path, struct stat *buf) override;

        pid_t waitpid(pid_t pid, int *status, int options) override;

        int execve(const char *filename, char *const argv[], char *const envp[]) override;

        pid_t fork(void) override;

        int kill(pid_t pid, int sig) override;

        int system(const char *command) override;

        FILE *fopen(const char *filename, const char *mode) override;

        int fclose(FILE *fp) override;

        DIR *opendir(const char *directory) override;

        int closedir(DIR *dir) override;

        struct dirent *readdir(DIR *directory) override;

        int remove(const char *filename) override;

        unsigned int sleep(unsigned int seconds) override;
    };

}
#endif //ROMI_ROVER_BUILD_AND_TEST_LINUX_H
