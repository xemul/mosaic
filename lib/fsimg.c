#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "mosaic.h"
#include "tessera.h"

struct fsimg_priv {
	int locfd;
};

static int open_fsimg(struct mosaic *m, int flags)
{
	struct fsimg_priv *fp;

	if (!m->default_fs)
		m->default_fs = strdup("ext4");

	fp = malloc(sizeof(*fp));
	fp->locfd = open(m->m_loc, O_DIRECTORY);
	if (fp->locfd < 0) {
		free(fp);
		return -1;
	}

	m->priv = fp;

	return 0;
}

static void release_fsimg(struct mosaic *m)
{
	struct fsimg_priv *fp = m->priv;

	close(fp->locfd);
	free(fp);
}

static int mount_fsimg(struct mosaic *m, const char *path, int mount_flags)
{
	return mount(m->m_loc, path, NULL, MS_BIND | mount_flags, NULL);
}


static int open_fsimg_tess(struct mosaic *m, struct tessera *t,
		int open_flags)
{
	struct fsimg_priv *fp = m->priv;
	struct stat b;

	return fstatat(fp->locfd, t->t_name, &b, 0);
}

static int new_fsimg_tess(struct mosaic *m,
		char *name, unsigned long size_in_blocks, char *fsname, int make_flags)
{
	struct fsimg_priv *fp = m->priv;
	int imgf;
	char mkfs_call[1024];

	imgf = openat(fp->locfd, name, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (imgf < 0)
		return -1;

	if (ftruncate(imgf, size_in_blocks << MOSAIC_BLOCK_SHIFT) < 0) {
		close(imgf);
		return -1;
	}

	if (fsname) {
		/*
		 * FIXME -- fork and exec mkfs
		 */
		sprintf(mkfs_call, "mkfs -t %s %s/%s", fsname, m->m_loc, name);
		if (system(mkfs_call)) {
			close(imgf);
			return -1;
		}
	}

	close(imgf);
	return 0;
}

static int drop_fsimg_tess(struct mosaic *m, struct tessera *t,
		int drop_flags)
{
	struct fsimg_priv *fp = m->priv;

	/*
	 * FIXME -- what if mounted?
	 */

	return unlinkat(fp->locfd, t->t_name, 0);
}

static int attach_fsimg_tess(struct mosaic *m, struct tessera *t,
		char *devs, int len, int flags)
{
	struct fsimg_priv *fp = m->priv;
	char aux[1024], *nl;
	FILE *lsp;

	/*
	 * FIXME: call losetup by hands?
	 */
	sprintf(aux, "losetup --find --show /proc/self/fd/%d/%s", fp->locfd, t->t_name);
	lsp = popen(aux, "r");
	if (!lsp)
		return -1;

	fgets(aux, sizeof(aux), lsp);
	pclose(lsp);

	nl = strchr(aux, '\n');
	if (nl)
		*nl = '\0';

	strcpy(devs, aux);
	return strlen(devs);
}

static int detach_fsimg_tess(struct mosaic *m, struct tessera *t, char *devs)
{
	char aux[1024];

	sprintf(aux, "losetup -d %s\n", devs);
	if (system(aux))
		return -1;

	return 0;
}

static int resize_fsimg_tess(struct mosaic *m, struct tessera *t,
		unsigned long size_in_blocks, int resize_flags)
{
	return -1;
}

const struct mosaic_ops mosaic_fsimg = {
	.open = open_fsimg,
	.release = release_fsimg,
	.mount = mount_fsimg,

	.open_tessera = open_fsimg_tess,
	.new_tessera = new_fsimg_tess,
	.clone_tessera = NULL, /* regular loops can't do it */
	.drop_tessera = drop_fsimg_tess,
	.attach_tessera = attach_fsimg_tess,
	.detach_tessera = detach_fsimg_tess,
	.resize_tessera = resize_fsimg_tess,
};
