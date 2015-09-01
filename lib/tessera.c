#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include "mosaic.h"
#include "tessera.h"
#include "uapi/mosaic.h"

tessera_t mosaic_open_tess(mosaic_t m, char *name, int open_flags)
{
	struct tessera *t;

	if (open_flags)
		return NULL;

	t = malloc(sizeof(*t));

	/*
	 * FIXME -- refcounting against mosaic_close()
	 */
	t->m = m;
	t->t_name = strdup(name);

	if (m->m_ops->open_tessera(m, t, open_flags)) {
		free(t);
		free(t->t_name);
		return NULL;
	}

	return t;
}

void mosaic_close_tess(tessera_t t)
{
	free(t->t_name);
	free(t);
}

int mosaic_make_tess(mosaic_t m, char *name, unsigned long size_in_blocks, int make_flags)
{
	if (make_flags)
		return -1;

	return m->m_ops->new_tessera(m, name, size_in_blocks, NULL, make_flags);
}

int mosaic_make_tess_fs(mosaic_t m, char *name, unsigned long size_in_blocks, int make_flags)
{
	if (make_flags)
		return -1;

	return m->m_ops->new_tessera(m, name, size_in_blocks, m->default_fs, make_flags);
}

int mosaic_clone_tess(tessera_t from, char *name, int clone_flags)
{
	struct mosaic *m = from->m;

	if (!m->m_ops->clone_tessera)
		return -1;
	if (clone_flags)
		return -1;

	return m->m_ops->clone_tessera(m, from, name, clone_flags);
}

int mosaic_drop_tess(tessera_t t, int drop_flags)
{
	struct mosaic *m = t->m;

	if (drop_flags)
		return -1;

	if (m->m_ops->drop_tessera(m, t, drop_flags))
		return -1;

	mosaic_close_tess(t);
	return 0;
}

int mosaic_mount_tess(tessera_t t, char *path, int mount_flags)
{
	struct mosaic *m = t->m;

	return m->m_ops->mount_tessera(m, t, path, mount_flags);
}

int mosaic_umount_tess(tessera_t t, char *path, int umount_flags)
{
	struct mosaic *m = t->m;

	if (umount_flags)
		return -1;

	if (m->m_ops->umount_tessera)
		return m->m_ops->umount_tessera(m, t, path, umount_flags);
	else
		return umount(path);
}

int mosaic_resize_tess(tessera_t t, unsigned long new_size_in_blocks, int resize_flags)
{
	struct mosaic *m = t->m;

	if (resize_flags)
		return -1;

	return m->m_ops->resize_tessera(m, t, new_size_in_blocks, resize_flags);
}
