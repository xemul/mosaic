#ifndef __MOSAIC_UTIL_H__
#define __MOSAIC_UTIL_H__
int scan_mounts(char *path, char *device);
int remove_rec(int dir_fd);
int get_subdir_size(int fd, unsigned long *sizep);
#endif
