#include <unistd.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include "mosaic.h"
#include "tessera.h"
#include "util.h"

static int open_plain(struct mosaic *m, int open_flags)
{
	if (m->default_fs)
		return -1;

	return open_mosaic_subdir(m);
}


static int new_plain_tess(struct mosaic *m, char *name,
		unsigned long size_in_blocks, int new_flags)
{
	struct mosaic_subdir_priv *pp = m->priv;

	if (!(new_flags & NEW_TESS_WITH_FS))
		return -1;

	/* FIXME -- size_in_blocks ignored */
	return mkdirat(pp->dir, name, 0600);
}

static int open_plain_tess(struct mosaic *m, struct tessera *t,
		int open_flags)
{
	struct mosaic_subdir_priv *pp = m->priv;
	struct stat b;

	return fstatat(pp->dir, t->t_name, &b, 0);
}

static int drop_plain_tess(struct mosaic *m, struct tessera *t,
		int drop_flags)
{
	struct mosaic_subdir_priv *pp = m->priv;
	int fd;

	fd = openat(pp->dir, t->t_name, O_DIRECTORY);
	if (fd < 0)
		return -1;

	if (remove_rec(fd))
		return -1;

	return unlinkat(pp->dir, t->t_name, AT_REMOVEDIR);
}

static int resize_plain_tess(struct mosaic *m, struct tessera *t,
		unsigned long size_in_blocks, int resize_flags)
{
	/* FIXME */
	return -1;
}

static int get_plain_size(struct mosaic *m, struct tessera *t,
		unsigned long *size_in_blocks)
{
	struct mosaic_subdir_priv *pp = m->priv;
	int fd, ret;

	fd = openat(pp->dir, t->t_name, O_DIRECTORY);
	if (fd < 0)
		return -1;

	 ret = get_subdir_size(fd, size_in_blocks);
	 close(fd);

	 *size_in_blocks >>= MOSAIC_BLOCK_SHIFT;

	 return ret;
}

const struct mosaic_ops mosaic_plain = {
	.init = init_mosaic_subdir,
	.open = open_plain,
	.release = release_mosaic_subdir,
	.mount = bind_mosaic_subdir_loc,

	.new_tessera = new_plain_tess,
	.open_tessera = open_plain_tess,
	.drop_tessera = drop_plain_tess,
	.resize_tessera = resize_plain_tess,
	.mount_tessera = bind_tess_loc,
	.get_tessera_size = get_plain_size,

	.parse_layout = parse_mosaic_subdir_layout,
};
