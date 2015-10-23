#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <stdio.h>
#include <limits.h>
#include "mosaic.h"
#include "volume.h"
#include "util.h"
#include "uapi/mosaic.h"

static volume_t alloc_vol(mosaic_t m, const char *name)
{
	struct volume *v;

	v = xzalloc(sizeof(*v));

	v->t_name = map_vol_name(m, name);
	if (!v->t_name) {
		// error is printed by map_vol_name()
		free(v);
		return NULL;
	}
	v->m = m;

	return v;
}

static void free_vol(volume_t v)
{
	free(v->t_name);
	free(v);
}

volume_t mosaic_open_vol(mosaic_t m, const char *name, int open_flags)
{
	volume_t v;

	/*
	 * FIXME -- refcounting against mosaic_close_vol()
	 * FIXME -- locking (so only one client can work with the volume)
	 */

	v = alloc_vol(m, name);
	if (!v)
		return NULL;

	// Use open_volume() if provided
	if (m->m_ops->open_volume) {
		if (m->m_ops->open_volume(m, v, open_flags) < 0) {
			free(v);
			return NULL;
		}
		return v;
	}

	// If no open_volume() is available, just check	the volume exists.
	if (m->m_ops->have_volume(m, v->t_name, open_flags) != 1) {
		loge("Volume %s not found\n", name);
		free_vol(v);
		return NULL;
	}

	return v;
}

void mosaic_close_vol(volume_t v)
{
	struct mosaic *m = v->m;

	if (m->m_ops->close_volume)
		m->m_ops->close_volume(m, v);

	free_vol(v);
}

int mosaic_have_vol(mosaic_t m, const char *name, int flags)
{
	char *newname;
	bool ret;

	if (flags) {
		loge("%s: unsupported flags 0x%x\n", __func__, flags);
		return -1;
	}

	newname = map_vol_name(m, name);
	if (!newname)
		return -1;

	ret = m->m_ops->have_volume(m, newname, flags);
	free(newname);
	return ret;
}

int mosaic_make_vol(mosaic_t m, const char *name,
		unsigned long size_in_blocks, int make_flags)
{
	char *newname;
	int ret;

	if (make_flags)
		return -1;

	newname = map_vol_name(m, name);
	if (!newname)
		return -1;

	ret = m->m_ops->new_volume(m, newname, size_in_blocks, make_flags);
	free(newname);

	return ret;
}

int mosaic_make_vol_fs(mosaic_t m, const char *name, unsigned long size_in_blocks, int make_flags)
{
	char *newname;
	int ret;

	if (make_flags)
		return -1;

	newname = map_vol_name(m, name);
	if (!newname)
		return -1;

	ret = m->m_ops->new_volume(m, newname, size_in_blocks,
			make_flags | NEW_VOL_WITH_FS);
	free(newname);

	return ret;
}

int mosaic_clone_vol(volume_t from, const char *name, int clone_flags)
{
	struct mosaic *m = from->m;
	char *newname;
	int ret;

	if (!m->m_ops->clone_volume)
		return -1;
	if (clone_flags)
		return -1;

	newname = map_vol_name(m, name);
	if (!newname)
		return -1;

	ret = m->m_ops->clone_volume(m, from, newname, clone_flags);
	free(newname);

	return ret;
}

int mosaic_drop_vol(volume_t t, int drop_flags)
{
	struct mosaic *m = t->m;

	if (drop_flags)
		return -1;

	if (m->m_ops->drop_volume(m, t, drop_flags))
		return -1;

	return 0;
}

int mosaic_mount_vol(volume_t t, const char *path, int mount_flags)
{
	struct mosaic *m = t->m;
	char tdev[1024];

	if (m->m_ops->mount_volume)
		return m->m_ops->mount_volume(m, t, path, mount_flags);

	if (!m->m_ops->attach_volume)
		return -1;

	if (m->m_ops->attach_volume(m, t, tdev, sizeof(tdev), 0) < 0)
		return -1;

	return mount(tdev, path, m->default_fs, mount_flags, NULL);
}

int mosaic_umount_vol(volume_t t, const char *path, int umount_flags)
{
	struct mosaic *m = t->m;
	int ret;

	if (umount_flags)
		return -1;

	if (m->m_ops->umount_volume)
		return m->m_ops->umount_volume(m, t, path, umount_flags);

	ret = umount(path);

	if (ret == 0 && m->m_ops->detach_volume)
		ret = m->m_ops->detach_volume(m, t);

	return ret;
}

int mosaic_get_vol_bdev(volume_t t, char *dev, int len, int flags)
{
	struct mosaic *m = t->m;

	if (flags)
		return -1;

	if (!m->m_ops->attach_volume)
		return -1;

	return m->m_ops->attach_volume(m, t, dev, len, flags);
}

int mosaic_put_vol_bdev(volume_t t)
{
	struct mosaic *m = t->m;

	if (!m->m_ops->detach_volume)
		return -1;

	return m->m_ops->detach_volume(m, t);
}

int mosaic_resize_vol(volume_t t, unsigned long new_size_in_blocks, int resize_flags)
{
	struct mosaic *m = t->m;

	if (resize_flags || !m->m_ops->resize_volume)
		return -1;

	return m->m_ops->resize_volume(m, t, new_size_in_blocks, resize_flags);
}

int mosaic_get_vol_size(volume_t t, unsigned long *size_in_blocks)
{
	struct mosaic *m = t->m;

	if (!size_in_blocks)
		return -1;

	return m->m_ops->get_volume_size(m, t, size_in_blocks);
}
