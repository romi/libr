#include "Linux.h"

namespace  rpp
{

    Linux::Linux()
    {
    }

    int Linux::open(const char *pathname, int flags)
    {
        return ::open(pathname, flags);
    }

    int Linux::close(int fd)
    {
        return ::close(fd);
    }


    void Linux::exit(int status)
    {
        return ::exit(status);
    }

    sighandler_t Linux::signal(int signum, sighandler_t handler)
    {
        return ::signal(signum, handler);
    }

    int Linux::stat(const char *path, struct stat *buf)
    {
        return ::stat(path, buf);
    }

    pid_t Linux::waitpid(pid_t pid, int *status, int options)
    {
        return ::waitpid(pid, status, options);
    }

    int Linux::execve(const char *filename, char *const argv[], char *const envp[])
    {
        return ::execve(filename, argv, envp);
    }

    pid_t Linux::fork(void)
    {
        return ::fork();
    }

    int Linux::kill(pid_t pid, int sig)
    {
        return ::kill(pid, sig);
    }

    int Linux::system(const char* command)
    {
        return ::system(command);
    }

    DIR * Linux::opendir(const char * directory)
    {
        return ::opendir( directory );
    }

    int Linux::closedir( DIR * dir )
    {
        return ::closedir( dir );
    }


    FILE * Linux::fopen( const char * file_name, const char * mode )
    {
        return ::fopen( file_name, mode );
    }

    int Linux::fclose(FILE *fp)
    {
        return ::fclose(fp);
    }

    struct dirent* Linux::readdir( DIR * directory )
    {
        return :: readdir( directory );
    }

    int Linux::remove( const char * filename )
    {
        return ::remove( filename );
    }

    unsigned int Linux::sleep(unsigned int seconds)
    {
        return ::sleep(seconds);
    }

}

