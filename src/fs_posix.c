#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "r.h"

int path_exists(const char *path)
{
        struct stat sb;
        if (stat(path, &sb) != 0)
                return 0;
        return 1;
}

int is_file(const char *path)
{
        struct stat sb;
        if (stat(path, &sb) != 0)
                return 0;
        if ((sb.st_mode & S_IFMT) == S_IFREG)
                return 1;
        return 0;
}

int is_directory(const char *path)
{
        struct stat sb;
        if (stat(path, &sb) != 0)
                return 0;
        if ((sb.st_mode & S_IFMT) == S_IFDIR)
                return 1;
        return 0;
}

int is_absolute(const char *path)
{
        return path[0] == '/'; // FIXME!
}

int make_absolute_path(const char *path, char *buffer, int len)
{
        int r;
        
        if (is_absolute(path)) {
                r = snprintf(buffer, len, "%s", path);
        } else {
                char *wd = getcwd(NULL, 0);
                if (wd == NULL) return -1;
                r = snprintf(buffer, len, "%s/%s", wd, path);
                free(wd);
        }
        
        buffer[len-1] = 0;
        return r > len;
}

list_t *dir_list(const char *path)
{
        DIR *d;
        struct dirent *dir;
        list_t *list = NULL;
        
        d = opendir(path);
        if (d == NULL)
                return NULL;

        while ((dir = readdir(d)) != NULL) {
                if (strcmp(dir->d_name, ".") == 0
                    || strcmp(dir->d_name, "..") == 0)
                        continue;
                char *s = r_strdup(dir->d_name);
                list = list_append(list, s);
        }
        closedir(d);
        return list;        
}

