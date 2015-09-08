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

	if (mosaic_parse_config(cfg, m))
		goto err;

	if (m->m_ops->open(m, open_flags))
		goto err;

	return m;

err:
	mosaic_close(m);
	return NULL;
}

void mosaic_close(mosaic_t m)
{
	if (m->m_ops) {
		if (m->m_ops->release)
			m->m_ops->release(m);
		if (m->default_fs)
			free(m->default_fs);
		if (m->layout)
			free(m->layout);
	}
	free(m);
}

int mosaic_mount(mosaic_t m, char *path, int mount_flags)
{
	return m->m_ops->mount(m, path, mount_flags);
}

int bind_mosaic_subdir_loc(struct mosaic *m, const char *path, int mount_flags)
{
	struct mosaic_subdir_priv *p = m->priv;
	char *src;

	src = p->fs_subdir ? : m->m_loc;
	return mount(src, path, NULL, MS_BIND | mount_flags, NULL);
}

int bind_tess_loc(struct mosaic *m, struct tessera *t,
		const char *path, int mount_flags)
{
	char aux[1024];

	sprintf(aux, "%s/%s", m->m_loc, t->t_name);
	return mount(aux, path, NULL, MS_BIND | mount_flags, NULL);
}

int init_mosaic_subdir(struct mosaic *m)
{
	struct mosaic_subdir_priv *p;

	p = malloc(sizeof(*p));
	p->dir = -1;
	p->fs_subdir = NULL;
	p->tess_subdir = NULL;
	m->priv = p;
	return 0;
}

int parse_mosaic_subdir_layout(struct mosaic *m, char *key, char *val)
{
	struct mosaic_subdir_priv *p = m->priv;

	if (!strcmp(key, "fs")) {
		int len;

		if (p->fs_subdir)
			free(p->fs_subdir);

		len = strlen(m->m_loc) + strlen(val) + 2;
		p->fs_subdir = malloc(len);
		sprintf(p->fs_subdir, "%s/%s", m->m_loc, val);
		free(val);
		return 0;
	}

	/* FIXME: implement */
#if 0
	if (!strcmp(key, "tess")) {
		if (p->tess_subdir)
			free(p->tess_subdir);
		p->tess_subdir = val;
		return 0;
	}
#endif

	return -1;
}

int open_mosaic_subdir(struct mosaic *m)
{
	struct mosaic_subdir_priv *p = m->priv;

	p->dir = open(m->m_loc, O_DIRECTORY);
	return p->dir >= 0 ? 0 : -1;
}

void release_mosaic_subdir(struct mosaic *m)
{
	struct mosaic_subdir_priv *p = m->priv;

	if (p->dir >= 0)
		close(p->dir);
	if (p->fs_subdir)
		free(p->fs_subdir);
	if (p->tess_subdir)
		free(p->tess_subdir);
	free(p);
}
