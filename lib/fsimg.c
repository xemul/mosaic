#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "mosaic.h"
#include "tessera.h"

static int open_fsimg(struct mosaic *m, int flags)
{
	if (!m->default_fs)
		m->default_fs = strdup("ext4");

	return open_mosaic_subdir(m);
}


static int open_fsimg_tess(struct mosaic *m, struct tessera *t,
		int open_flags)
{
	struct mosaic_subdir_priv *fp = m->priv;
	struct stat b;

	return fstatat(fp->m_loc_dir, t->t_name, &b, 0);
}

static int new_fsimg_tess(struct mosaic *m, char *name,
		unsigned long size_in_blocks, int make_flags)
{
	struct mosaic_subdir_priv *fp = m->priv;
	int imgf;
	char mkfs_call[1024];

	/*
	 * FIXME -- add separate subdir for images and separate for
	 * regular fs mount?
	 */

	imgf = openat(fp->m_loc_dir, name, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (imgf < 0)
		return -1;

	if (ftruncate(imgf, size_in_blocks << MOSAIC_BLOCK_SHIFT) < 0) {
		close(imgf);
		return -1;
	}

	if (make_flags & NEW_TESS_WITH_FS) {
		/*
		 * FIXME -- fork and exec mkfs
		 */
		sprintf(mkfs_call, "mkfs -t %s -F %s/%s", m->default_fs, m->m_loc, name);
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
	struct mosaic_subdir_priv *fp = m->priv;

	/*
	 * FIXME -- what if mounted?
	 */

	return unlinkat(fp->m_loc_dir, t->t_name, 0);
}

static int attach_fsimg_tess(struct mosaic *m, struct tessera *t,
		char *devs, int len, int flags)
{
	struct mosaic_subdir_priv *fp = m->priv;
	char aux[1024], *nl;
	FILE *lsp;

	/*
	 * FIXME: call losetup by hands?
	 * FIXME: multiple calls should report the same device?
	 */
	sprintf(aux, "losetup --find --show /proc/self/fd/%d/%s", fp->m_loc_dir, t->t_name);
	lsp = popen(aux, "r");
	if (!lsp)
		return -1;

	fgets(aux, sizeof(aux), lsp);
	pclose(lsp);

	nl = strchr(aux, '\n');
	if (nl)
		*nl = '\0';

	strncpy(devs, aux, len);
	return strlen(aux);
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
	/* FIXME */
	return -1;
}

static int get_fsimg_size(struct mosaic *m, struct tessera *t, unsigned long *size_in_blocks)
{
	struct mosaic_subdir_priv *fp = m->priv;
	struct stat buf;

	if (fstatat(fp->m_loc_dir, t->t_name, &buf, 0))
		return -1;

	*size_in_blocks = buf.st_size >> MOSAIC_BLOCK_SHIFT;
	return 0;
}

const struct mosaic_ops mosaic_fsimg = {
	.name = "fsimg",
	.open = open_fsimg,
/*	.mount = FIXME: location can be device */

	.open_tessera = open_fsimg_tess,
	.new_tessera = new_fsimg_tess,
	.drop_tessera = drop_fsimg_tess,
	.attach_tessera = attach_fsimg_tess,
	.detach_tessera = detach_fsimg_tess,
	.resize_tessera = resize_fsimg_tess,
	.get_tessera_size = get_fsimg_size,

	.parse_layout = parse_mosaic_subdir_layout,
};
