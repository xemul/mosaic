#include <unistd.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include "mosaic.h"
#include "tessera.h"
#include "util.h"

struct plain_priv {
	int locfd;
};

static int open_plain(struct mosaic *m, int open_flags)
{
	struct plain_priv *pp;

	if (m->default_fs)
		return -1;

	pp = malloc(sizeof(*pp));
	pp->locfd = open(m->m_loc, O_DIRECTORY);
	if (pp->locfd < 0) {
		free(pp);
		return -1;
	}

	m->priv = pp;
	return 0;
}

static void release_plain(struct mosaic *m)
{
	struct plain_priv *pp = m->priv;

	close(pp->locfd);
	free(pp);
}

static int new_plain_tess(struct mosaic *m, char *name,
		unsigned long size_in_blocks, int new_flags)
{
	struct plain_priv *pp = m->priv;
	/* FIXME -- size_in_blocks ignored */
	return mkdirat(pp->locfd, name, 0600);
}

static int open_plain_tess(struct mosaic *m, struct tessera *t,
		int open_flags)
{
	struct plain_priv *pp = m->priv;
	struct stat b;

	return fstatat(pp->locfd, t->t_name, &b, 0);
}

static int drop_plain_tess(struct mosaic *m, struct tessera *t,
		int drop_flags)
{
	struct plain_priv *pp = m->priv;

	if (remove_rec(dup(pp->locfd)))
		return -1;

	return unlinkat(pp->locfd, t->t_name, AT_REMOVEDIR);
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
	.release = release_plain,
	.mount = bind_mosaic_loc,

	.new_tessera = new_plain_tess,
	.open_tessera = open_plain_tess,
	.drop_tessera = drop_plain_tess,
	.resize_tessera = resize_plain_tess,
	.mount_tessera = mount_plain_tess,
};
