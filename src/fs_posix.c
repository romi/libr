/*
  ROMI libr

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  The libr library provides some hardware abstractions and low-level
  utility functions.

  libr is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include "r.h"

int path_exists(const char *path)
{
        struct stat sb;
        if (stat_wrapper(path, &sb) != 0)
                return 0;
        return 1;
}

int is_file(const char *path)
{
        struct stat sb;
        if (stat_wrapper(path, &sb) != 0)
                return 0;
        if (S_ISREG(sb.st_mode))
 //       if ((sb.st_mode & S_IFMT) == S_IFREG)
                return 1;
        return 0;
}

int is_directory(const char *path)
{
        struct stat sb;
        if (stat_wrapper(path, &sb) != 0)
                return 0;
        if(S_ISDIR(sb.st_mode))
     //   if ((sb.st_mode & S_IFMT) == S_IFDIR)
                return 1;
        return 0;
}

int path_is_absolute(const char *path)
{
        return path[0] == '/'; // FIXME!
}

int path_make_absolute(const char *path, char *buffer, int len)
{
        int r;
        
        if (path_is_absolute(path)) {
                r = snprintf(buffer, len, "%s", path);
        } else {
                char *wd = getcwd_wrapper(NULL, 0);
                if (wd == NULL) return -1;
                r = snprintf(buffer, len, "%s/%s", wd, path);
                free(wd);
        }
        
        buffer[len-1] = 0;
        return r > len;
}

list_t *directory_list(const char *path)
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

static int _directory_create(const char *path)
{
        /* r_debug("_directory_create %s", path); */
        if (path_exists(path)) {
                if (is_directory(path))
                        return 0;
                else {
                        r_err("directory_create: Path exists but is not a directory: %s",
                                path);
                        return -1;
                }
        }
        int err = mkdir(path, 0777);
        if (err != 0) {
                char msg[200];
                strerror_r(errno, msg, sizeof(msg));
                r_err("Failed to create directory %s: %s", path, msg);
        }
        return err;
}

int directory_create(const char *path)
{
        r_debug("directory_create %s", path);
        if (path_exists(path)) {
                if (is_directory(path))
                        return 0;
                else {
                        r_err("directory_create: Path exists but is not a directory: %s",
                                path);
                        return -1;
                }
        }
        list_t* dirs = path_break(path);
        
        /* for (list_t* l = dirs; l != NULL; l = list_next(l)) { */
        /*         const char *s = list_get(l, char); */
        /*         r_debug("directory_create, dir %s", s); */
        /* } */
        
        list_t* parents = NULL;
        for (list_t* l = dirs; l != NULL; l = list_next(l)) {
                char *s = list_get(l, char);
                parents = list_append(parents, s);
                
                char buffer[1024];
                path_glue(parents, 0, buffer, sizeof(buffer));
                if (_directory_create(buffer) != 0) {
                        path_delete(dirs);
                        delete_list(parents);
                        return -1;
                }
        }
        
        path_delete(dirs);
        delete_list(parents);
        return 0;
}

list_t* path_break(const char *path)
{
        list_t* elements = NULL;
        membuf_t *buf = new_membuf();
        if (buf == NULL)
                return NULL;

        if (path[0] == '/')
                membuf_append(buf, "/", 1);

        for (int i = 0; path[i] != 0; i++) {
                char c = path[i];
                if (c == '/') {
                        if (membuf_len(buf) > 0) {
                                membuf_append_zero(buf);
                                char *s = r_strdup(membuf_data(buf));
                                elements = list_append(elements, s);
                                membuf_clear(buf);
                        } 
                } else {
                        membuf_append(buf, &c, 1);
                }
        }
        if (membuf_len(buf) > 0) {
                membuf_append_zero(buf);
                char *s = r_strdup(membuf_data(buf));
                elements = list_append(elements, s);
        }
        delete_membuf(buf);
        return elements;
}

int path_glue(list_t* elements, int absolute, char *buffer, int len)
{
        const char *s = list_get(elements, char);
        elements = list_next(elements);
        
        if (rstreq(s, "/") && elements == NULL) {
                rprintf(buffer, len, "/");
                return 0;
        }
        
        membuf_t *buf = new_membuf();
        if (buf == NULL) return -1;
        
        if (rstreq(s, "/") && elements != NULL) {
                ; // do nothing and continue
        } else if (absolute) {
                membuf_append_str(buf, "/");
                membuf_append_str(buf, s);
        } else {
                membuf_append_str(buf, s);
        }
        
        while (elements != NULL) {
                const char *s = list_get(elements, char);
                membuf_append(buf, "/", 1);
                membuf_append_str(buf, s);
                elements = list_next(elements);
        }
        
        membuf_append_zero(buf);
        rprintf(buffer, len, "%s", membuf_data(buf));
        delete_membuf(buf);

        return 0;
}

void path_delete(list_t* list)
{
        for (list_t* l = list; l != NULL; l = list_next(l)) {
                char *s = list_get(l, char);
                if (s) r_free(s);
        }
        delete_list(list);
}

static int get_user_info(const char *user, uid_t *uid, gid_t *gid) 
{
        struct passwd pwbuf;
        struct passwd *pwbufp = NULL;
        char *buf;
        long bufsize;

        // From man getpwnam_r
        bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
        if (bufsize == -1)
                bufsize = 16384;

        buf = malloc((size_t)bufsize);
        if (buf == NULL) {
                r_panic("out of memory");
                return -1;
        }

        int err = getpwnam_r(user, &pwbuf, buf, bufsize, &pwbufp);
        if (pwbufp == NULL) {
                if (err == 0) {
                        r_err("Failed to obtain the UID associated with '%s'", user);
                        free(buf);
                        return -1;
                } else {
                        char msg[200];
                        strerror_r(err, msg, 200);
                        r_err("getpwnam_r failed: %s", msg);
                        free(buf);
                        return -1;
                }
        }
        free(buf);

        *uid = pwbuf.pw_uid;
        *gid = pwbuf.pw_gid;
        return 0;
}

int path_chown(const char *path, const char *user)
{
        uid_t uid;
        gid_t gid;
        
        if (get_user_info(user, &uid, &gid) != 0) {
                r_err("Failed to obtain user info for user '%s'", user);
                return -1;
        }
                
        return chown(path, uid, gid);
}

static int _file_lock(int fd)
{
        struct flock lock;
        lock.l_type = F_WRLCK;
        lock.l_start = 0;
        lock.l_whence = SEEK_SET;
        lock.l_len = 0;
        lock.l_pid = 0;

        while (fcntl(fd, F_SETLKW, &lock) == -1) {
                if (errno != EINTR) {
                        char msg[200];
                        strerror_r(errno, msg, sizeof(msg));
                        r_err("Failed to lock the file: %s", msg);
                        return -1;
                }
        }
        return 0;
}

int _file_backup(const char *path)
{
        char backup[1024];
        rprintf(backup, sizeof(backup), "%s.backup", path);
        if (rename(path, backup) != 0) {
                if (errno != ENOENT) {
                        r_warn("Failed to create a backup file");
                        return -1;
                }
        }
        return 0;
}

int _file_unlock(int fd)
{
        struct flock lock;
        lock.l_type = F_UNLCK;
        lock.l_start = 0;
        lock.l_whence = SEEK_SET;
        lock.l_len = 0;
        lock.l_pid = 0;

        if (fcntl(fd, F_SETLK, &lock) == -1) {
                r_warn("Unlock failed");
                return -1;
        }
        return 0;
}

int _file_store(const char *path, int fd, char *data, int32_t len)
{
        int32_t written = 0;
        while (written < len) {
                int32_t n = write(fd, data + written, len - written);
                if (n == -1) {
                        char msg[200];
                        strerror_r(errno, msg, sizeof(msg));
                        r_err("Failed to write the file %s: %s", path, msg);
                        return -1;
                }
                written += n;
        }
        return 0;
}

int file_store(const char *path, char *data, int len, int flags)
{
        int err = -1;
        int fd = -1;

        fd = open(path, O_WRONLY | O_CREAT, 0666);
        if (fd == -1) {
                char msg[200];
                strerror_r(errno, msg, sizeof(msg));
                r_err("Failed to open %s: %s", path, msg);
                return -1;
        }

        if ((flags & FS_LOCK) && _file_lock(fd) != 0)
                goto close_and_exit;

        if ((flags & FS_BACKUP) && _file_backup(path) != 0)
                goto unlock_close_and_exit;
        
        err = _file_store(path, fd, data, len);
        
unlock_close_and_exit:
        
        if ((flags & FS_LOCK) && _file_unlock(fd) != 0)
                err = -1;
        
close_and_exit:
        
        close(fd);

        return err;
}
