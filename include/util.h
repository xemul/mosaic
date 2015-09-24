#ifndef __MOSAIC_UTIL_H__
#define __MOSAIC_UTIL_H__
int scan_mounts(char *path, char *device);
int remove_rec(int dir_fd);
int get_subdir_size(int fd, unsigned long *sizep);

int copy_file(int src_dirfd, const char *src_dir,
		int dst_dirfd, const char *dst_dir,
		const char *name);

char *read_val(int dirfd, const char *dir, const char *name);

int write_val(int dirfd, const char *dir,
		const char *name, const char *val);

#endif
