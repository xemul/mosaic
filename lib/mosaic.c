#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>

#include "mosaic.h"
#include "tessera.h"
#include "log.h"
#include "uapi/mosaic.h"

const struct mosaic_ops *mosaic_find_ops(char *type)
{
	if (!strcmp(type, "fsimg"))
		return &mosaic_fsimg;
	if (!strcmp(type, "btrfs"))
		return &mosaic_btrfs;
	if (!strcmp(type, "plain"))
		return &mosaic_plain;

	return NULL;
}

mosaic_t mosaic_open(const char *cfg, int open_flags)
{
	struct mosaic *m;

	if (open_flags)
		return NULL;

	m = malloc(sizeof(*m));
	memset(m, 0, sizeof(*m));

	if (mosaic_parse_config(cfg, m)) {
		free(m);
		return NULL;
	}

	if (m->m_ops->open(m, open_flags)) {
		free(m);
		return NULL;
	}

	return m;
}

void mosaic_close(mosaic_t m)
{
	if (m->m_ops->release)
		m->m_ops->release(m);
	if (m->default_fs)
		free(m->default_fs);
	if (m->layout)
		free(m->layout);
	free(m);
}

int mosaic_mount(mosaic_t m, char *path, int mount_flags)
{
	return m->m_ops->mount(m, path, mount_flags);
}

int bind_mosaic_loc(struct mosaic *m, const char *path, int mount_flags)
{
	return mount(m->m_loc, path, NULL, MS_BIND | mount_flags, NULL);
}

int bind_tess_loc(struct mosaic *m, struct tessera *t,
		const char *path, int mount_flags)
{
	char aux[1024];

	sprintf(aux, "%s/%s", m->m_loc, t->t_name);
	return mount(aux, path, NULL, MS_BIND | mount_flags, NULL);
}

int open_mosaic_subdir(struct mosaic *m)
{
	struct mosaic_subdir_priv *p;

	p = malloc(sizeof(*p));
	p->dir = open(m->m_loc, O_DIRECTORY);
	if (p->dir < 0) {
		free(p);
		return -1;
	}

	m->priv = p;
	return 0;
}

void release_mosaic_subdir(struct mosaic *m)
{
	struct mosaic_subdir_priv *p = m->priv;

	close(p->dir);
	free(p);
}
