#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
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
