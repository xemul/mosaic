#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include "mosaic.h"
#include "tessera.h"

static int open_btrfs(struct mosaic *m, int flags)
{
	if (m->default_fs)
		return -1;

	m->default_fs = strdup("btrfs");
	return 0;
}

static int new_btrfs_subvol(struct mosaic *m, char *name,
		unsigned long size_in_blocks, int make_flags)
{
	char aux[1024];

	if (!(make_flags & NEW_TESS_WITH_FS))
		return -1;

	/*
	 * FIXME: locate tesserae subvolumes in subdirectories
	 * FIXME: qgroups
	 */
	sprintf(aux, "btrfs subvolume create %s/%s", m->m_loc, name);
	if (system(aux))
		return -1;

	return 0;
}

static int open_btrfs_subvol(struct mosaic *m, struct tessera *t,
		int open_flags)
{
	return 0; /* FIXME: check it exists */
}

static int clone_btrfs_subvol(struct mosaic *m, struct tessera *from,
		char *name, int clone_flags)
{
	char aux[1024];

	/*
	 * FIXME: locate tesserae subvolumes in subdirectories
	 */
	sprintf(aux, "btrfs subvolume snapshot %s/%s %s/%s",
			m->m_loc, from->t_name, m->m_loc, name);
	if (system(aux))
		return -1;

	return 0;
}

static int drop_btrfs_subvol(struct mosaic *m, struct tessera *t,
		int drop_flags)
{
	char aux[1024];

	/*
	 * FIXME: locate tesserae subvolumes in subdirectories
	 */
	sprintf(aux, "btrfs subvolume delete %s/%s", m->m_loc, t->t_name);
	if (system(aux))
		return -1;

	return 0;
}

static int resize_btrfs_subvol(struct mosaic *m, struct tessera *t,
		unsigned long size_in_blocks, int resize_flags)
{
	/* FIXME: qgroups */
	return -1;
}

static int get_btrfs_subvol_size(struct mosaic *m, struct tessera *t,
		unsigned long *size_in_blocks)
{
	/* FIXME: qgroups */
	return -1;
}

const struct mosaic_ops mosaic_btrfs = {
	.name = "btrfs",

	.open = open_btrfs,

	.new_tessera = new_btrfs_subvol,
	.open_tessera = open_btrfs_subvol,
	.clone_tessera = clone_btrfs_subvol,
	.drop_tessera = drop_btrfs_subvol,
	.mount_tessera = bind_tess_loc,
	.resize_tessera = resize_btrfs_subvol,
	.get_tessera_size = get_btrfs_subvol_size,

	.parse_layout = parse_mosaic_subdir_layout,
};
