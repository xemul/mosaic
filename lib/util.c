#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sendfile.h>

#include "util.h"

int scan_mounts(char *mpath, char *device)
{
	FILE *f;
	char mi[1024];

	f = fopen("/proc/self/mountstats", "r");
	if (!f)
		return -1;

	while (fgets(mi, sizeof(mi), f)) {
		char *dev, *path, *e;

		dev = mi + sizeof("device");
		e = strchr(dev, ' ');
		*e = '\0';

		path = e + 1 + sizeof("mounted on");
		e = strchr(path, ' ');
		*e = '\0';

		if (strcmp(path, mpath))
			continue;

		strcpy(device, dev);
		fclose(f);
		return 0;
	}

	fclose(f);
	return -1;
}

int remove_rec(int dir_fd)
{
	DIR *d;
	struct dirent *de;

	d = fdopendir(dir_fd);
	while ((de = readdir(d)) != NULL) {
		int flg = 0;

		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;

		if (de->d_type == DT_DIR) {
			int sub_fd;

			/* FIXME -- limit of descriptors may not be enough */
			sub_fd = openat(dir_fd, de->d_name, O_DIRECTORY);
			if (sub_fd < 0)
				goto err;

			if (remove_rec(sub_fd) < 0)
				goto err;

			flg = AT_REMOVEDIR;
		}

		if (unlinkat(dir_fd, de->d_name, flg))
			goto err;
	};

	closedir(d);
	return 0;

err:
	closedir(d);
	return -1;
}

int get_subdir_size(int fd, unsigned long *sizep)
{
	/* FIXME: implement */
	return -1;
}

int copy_file(int src_dirfd, const char *src_dir,
		int dst_dirfd, const char *dst_dir,
		const char *name)
{
	int ifd, ofd;
	ssize_t ret, s;
	struct stat st;

	if (fstatat(src_dirfd, name, &st, 0) < 0) {
		fprintf(stderr, "%s: can't stat %s/%s: %m\n",
				__func__, src_dir, name);
		return -1;
	}
	s = st.st_size;

	ifd = openat(src_dirfd, name, O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr, "%s: can't open %s/%s: %m\n",
				__func__, src_dir, name);
		return -1;
	}

	ofd = openat(dst_dirfd, name, O_RDWR | O_CREAT | O_EXCL, st.st_mode);
	if (ofd < 0) {
		fprintf(stderr, "%s: can't create %s/%s: %m\n",
				__func__, dst_dir, name);
		close(ifd);
		return -1;
	}
	fchown(ofd, st.st_uid, st.st_gid);

	ret = sendfile(ofd, ifd, NULL, s);
	close(ifd);
	close(ofd);

	if (ret < 0) {
		fprintf(stderr, "%s: sendfile %s/%s -> %s failed: %m\n",
				__func__, src_dir, name, dst_dir);
		return -1;
	} else if (ret != s) {
		fprintf(stderr, "%s: sendfile %s/%s -> %s short write (%zd/%zd)\n",
				__func__, src_dir, name, dst_dir, ret, s);
		return -1;
	}

	return 0;
}

const ssize_t max_val_len = 1024;

int write_val(int dirfd, const char *dir,
		const char *name, const char *val)
{
	int fd, ret = 0;
	ssize_t len;

	len = strlen(val) + 1; // +1 for \0
	if (len > max_val_len) {
		fprintf(stderr, "%s: variable %s too long (%zd > %zd)!\n",
				__func__, name, len, max_val_len);
		return -1;
	}

	fd = openat(dirfd, name, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "%s: can't open %s/%s: %m\n",
				__func__, dir, name);
		return -1;
	}
	if (write(fd, val, len) != len) {
		fprintf(stderr, "%s: can't read %s/%s: %m\n",
				__func__, dir, name);
		ret = -1;
	}
	if (close(fd) < 0) {
		fprintf(stderr, "%s: can't close %s/%s: %m\n",
				__func__, dir, name);
		ret = -1;
	}

	return ret;
}

char *read_val(int dirfd, const char *dir, const char *name)
{
	int fd;
	struct stat st;
	char *buf = NULL;
	ssize_t s;

	if (fstatat(dirfd, name, &st, 0) < 0) {
		if (errno != ENOENT)
			fprintf(stderr, "%s: can't stat %s/%s: %m\n",
					__func__, dir, name);
		return NULL;
	}
	s = st.st_size;

	fd = openat(dirfd, name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "%s: can't open %s/%s: %m\n",
				__func__, dir, name);
		return NULL;
	}

	if (s > max_val_len) {
		fprintf(stderr, "%s: file %s/%s size too large (%zd > %zd)\n",
				__func__, dir, name, s, max_val_len);
		goto out;
	}

	buf = malloc(s);
	if (!buf) {
		fprintf(stderr, "%s: can't allocate %zdb memory: %m\n",
				__func__, s);
		goto out;
	}
	if (read(fd, &buf, s) != s) {
		fprintf(stderr, "%s: can't read %s/%s: %m\n",
				__func__, dir, name);
		free(buf);
		buf = NULL;
	}

out:
	close(fd);
	return buf;
}

#ifdef DOTEST
int main(int argc, char **argv)
{
	int fd;

	fd = open(argv[1], O_DIRECTORY);
	if (fd < 0)
		return -1;

	return remove_rec(fd);
}
#endif
