#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "moctl.h"
#include "mosaic.h"

#define DEFAULT_LOC_PERMS	0600 /* FIXME */

static int create_subdir_mosaic(struct mosaic *m, int argc, char **argv)
{
	struct mosaic_subdir_priv *p = m->priv;

	if (mkdir(m->m_loc, DEFAULT_LOC_PERMS)) {
		perror("Can't make directory");
		return -1;
	}

	if (p->fs_subdir) {
		if (mkdir(p->fs_subdir, DEFAULT_LOC_PERMS)) {
			perror("Can't make fs subdir");
			return -1;
		}
	}

	return 0;
}

int create_fsimg(struct mosaic *m, int argc, char **argv)
{
	return create_subdir_mosaic(m, argc, argv);
}

int create_plain(struct mosaic *m, int argc, char **argv)
{
	return create_subdir_mosaic(m, argc, argv);
}

int create_btrfs(struct mosaic *m, int argc, char **argv)
{
	FILE *f;
	char aux[1024], *dev;

	if (argc < 2) {
		printf("Usage: moctl <name> create d:<device>|f:<imgfile>\n");
		return -1;
	}

	dev = argv[1] + 2;
	sprintf(aux, "mkfs -t btrfs %s", dev);
	f = popen(aux, "r");
	if (!f) {
		perror("Can't call mkfs");
		return -1;
	}

	pclose(f);

	if (mkdir(m->m_loc, DEFAULT_LOC_PERMS)) {
		perror("Can't make mosaic location");
		return -1;
	}

	if (argv[1][0] == 'f') {
		char *nl;

		/* FIXME -- stub stderr */
		sprintf(aux, "losetup --find --show %s", dev);
		f = popen(aux, "r");
		if (!f) {
			perror("Can't losetup");
			return -1;
		}

		fgets(aux, sizeof(aux), f);
		pclose(f);

		nl = strchr(aux, '\n');
		if (nl)
			*nl = '\0';

		dev = aux;
	}

	if (mount(dev, m->m_loc, "btrfs", 0, NULL)) {
		char aux2[1024];
		sprintf(aux2, "losetup -d %s", dev);
		system(aux2);
		perror("Can't mount btrfs");
		return -1;
	}

	return 0;
}
