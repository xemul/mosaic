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

	return open_locfd(m);
}


static int new_plain_tess(struct mosaic *m, char *name,
		unsigned long size_in_blocks, int new_flags)
{
	struct locfd_priv *pp = m->priv;

	if (!(new_flags & NEW_TESS_WITH_FS))
		return -1;

	/* FIXME -- size_in_blocks ignored */
	return mkdirat(pp->dir, name, 0600);
}

static int open_plain_tess(struct mosaic *m, struct tessera *t,
		int open_flags)
{
	struct locfd_priv *pp = m->priv;
	struct stat b;

	return fstatat(pp->dir, t->t_name, &b, 0);
}

static int drop_plain_tess(struct mosaic *m, struct tessera *t,
		int drop_flags)
{
	struct locfd_priv *pp = m->priv;
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

static int mount_plain_tess(struct mosaic *m, struct tessera *t,
		char *path, int mount_flags)
{
	char aux[1024];

	sprintf(aux, "%s/%s", m->m_loc, t->t_name);
	return mount(aux, path, NULL, MS_BIND | mount_flags, NULL);
}

const struct mosaic_ops mosaic_plain = {
	.open = open_plain,
	.release = release_locfd,
	.mount = bind_mosaic_loc,

	.new_tessera = new_plain_tess,
	.open_tessera = open_plain_tess,
	.drop_tessera = drop_plain_tess,
	.resize_tessera = resize_plain_tess,
	.mount_tessera = mount_plain_tess,
};
