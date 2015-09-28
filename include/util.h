#ifndef __MOSAIC_UTIL_H__
#define __MOSAIC_UTIL_H__
int scan_mounts(char *path, char *device);
int remove_rec(int dir_fd);
int get_subdir_size(int fd, unsigned long *sizep);

int copy_file(int src_dirfd, const char *src_dir,
		int dst_dirfd, const char *dst_dir,
		const char *name);

char *read_var(int dirfd, const char *dir, const char *name);

int write_var(int dirfd, const char *dir,
		const char *name, const char *val);

int run_prg(char *const argv[]);
#define HIDE_STDOUT	1 << 0	/* hide process' stdout */
#define HIDE_STDERR	1 << 1	/* hide process' stderr */
int run_prg_rc(char *const argv[], int hide_mask, int *rc);

#endif
