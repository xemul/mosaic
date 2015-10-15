#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <regex.h>
#include <limits.h>

#include "mosaic.h"
#include "volume.h"
#include "log.h"
#include "util.h"
#include "uapi/mosaic.h"

#ifndef MOSAIC_CONFIG_DIR
#define MOSAIC_CONFIG_DIR	"/etc/mosaic.d"
#endif

const struct mosaic_ops *mosaic_find_ops(char *type)
{
	if (!strcmp(type, "fsimg"))
		return &mosaic_fsimg;
	if (!strcmp(type, "btrfs"))
		return &mosaic_btrfs;
	if (!strcmp(type, "plain"))
		return &mosaic_plain;
	if (!strcmp(type, "ploop"))
		return &mosaic_ploop;

	return NULL;
}

static const char *name_to_config(const char *name, char *buf, int blen)
{
	if (name[0] == '.' || name[0] == '/')
		return name;

	snprintf(buf, blen, MOSAIC_CONFIG_DIR "/%s.mos", name);
	return buf;
}

mosaic_t mosaic_open(const char *name, int open_flags)
{
	const char *cfg;
	char aux[PATH_MAX];
	struct mosaic *m;

	if (open_flags)
		return NULL;

	m = malloc(sizeof(*m));
	memset(m, 0, sizeof(*m));

	cfg = name_to_config(name, aux, sizeof(aux));
	if (mosaic_parse_config(cfg, m))
		goto err;

	if (m->m_ops->open && m->m_ops->open(m, open_flags))
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
		else
			release_mosaic_subdir(m);
		if (m->default_fs)
			free(m->default_fs);
	}
	free_vol_map(m);
	free(m);
}

int mosaic_mount(mosaic_t m, const char *path, int mount_flags)
{
	if (m->m_ops->mount)
		return m->m_ops->mount(m, path, mount_flags);
	else
		return bind_mosaic_subdir_loc(m, path, mount_flags);
}

int bind_mosaic_subdir_loc(struct mosaic *m, const char *path, int mount_flags)
{
	struct mosaic_subdir_priv *p = m->priv;
	char *src;

	src = p->fs_subdir ? : m->m_loc;
	return mount(src, path, NULL, MS_BIND | mount_flags, NULL);
}

int bind_vol_loc(struct mosaic *m, struct volume *t,
		const char *path, int mount_flags)
{
	char aux[1024];

	snprintf(aux, sizeof(aux), "%s/%s", m->m_loc, t->t_name);
	return mount(aux, path, NULL, MS_BIND | mount_flags, NULL);
}

int init_mosaic_subdir(struct mosaic *m)
{
	struct mosaic_subdir_priv *p;

	p = malloc(sizeof(*p));
	p->m_loc_dir = -1;
	p->fs_subdir = NULL;
	m->priv = p;
	return 0;
}

int parse_mosaic_subdir_layout(struct mosaic *m, char *key, char *val)
{
	struct mosaic_subdir_priv *p = m->priv;

	if (!strcmp(key, "fs")) {
		int len;

		CHKVAL(key, val);
		CHKDUP(key, p->fs_subdir);

		len = strlen(m->m_loc) + strlen(val) + 2;
		p->fs_subdir = malloc(len);
		sprintf(p->fs_subdir, "%s/%s", m->m_loc, val);
		free(val);
		return 0;
	}

	/* FIXME: implement? */
#if 0
	if (!strcmp(key, "vol")) {
		if (p->vol_subdir)
			free(p->vol_subdir);
		p->vol_subdir = val;
		return 0;
	}
#endif

	loge("%s: unknown layout element: %s\n", __func__, key);

	return -1;
}

int open_mosaic_subdir(struct mosaic *m)
{
	struct mosaic_subdir_priv *p = m->priv;

	p->m_loc_dir = open(m->m_loc, O_DIRECTORY);
	return p->m_loc_dir >= 0 ? 0 : -1;
}

void release_mosaic_subdir(struct mosaic *m)
{
	struct mosaic_subdir_priv *p = m->priv;

	if (p->m_loc_dir >= 0)
		close(p->m_loc_dir);
	if (p->fs_subdir)
		free(p->fs_subdir);
	free(p);
}

int mosaic_get_features(struct mosaic *m, unsigned long long *feat)
{
	*feat = 0;

	if (m->m_ops->clone_volume)
		*feat |= MOSAIC_FEATURE_CLONE;
	if (m->m_ops->attach_volume)
		*feat |= MOSAIC_FEATURE_BDEV;
	if (m->m_ops->resize_volume)
		*feat |= MOSAIC_FEATURE_DISK_SIZE_MGMT;

	return 0;
}
