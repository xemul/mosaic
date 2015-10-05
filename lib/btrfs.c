#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <limits.h>
#include "mosaic.h"
#include "volume.h"
#include "util.h"

static int open_btrfs(struct mosaic *m, int flags)
{
	if (m->default_fs)
		return -1;

	m->default_fs = strdup("btrfs");
	return 0;
}

static int new_btrfs_subvol(struct mosaic *m, const char *name,
		unsigned long size_in_blocks, int make_flags)
{
	char *argv[8];
	char vol[PATH_MAX];
	int i;

	if (!(make_flags & NEW_VOL_WITH_FS))
		return -1;

	/*
	 * FIXME: locate this volume's subvolumes in subdirectories
	 * FIXME: qgroups
	 */

	i = 0;
	argv[i++] = "btrfs";
	argv[i++] = "subvolume";
	argv[i++] = "create";
	snprintf(vol, sizeof(vol), "%s/%s", m->m_loc, name);
	argv[i++] = vol;
	argv[i++] = NULL;
	if (run_prg(argv))
		return -1;

	return 0;
}

static int open_btrfs_subvol(struct mosaic *m, struct volume *t,
		int open_flags)
{
	return 0; /* FIXME: check it exists */
}

static int clone_btrfs_subvol(struct mosaic *m, struct volume *from,
		const char *name, int clone_flags)
{
	char *argv[8];
	char vol[PATH_MAX], pvol[PATH_MAX];
	int i;

	/*
	 * FIXME: locate subvolumes in subdirectories
	 */
	i = 0;
	argv[i++] = "btrfs";
	argv[i++] = "subvolume";
	argv[i++] = "snapshot";
	snprintf(pvol, sizeof(pvol), "%s/%s", m->m_loc, from->t_name);
	argv[i++] = pvol;
	snprintf(vol, sizeof(vol), "%s/%s", m->m_loc, name);
	argv[i++] = vol;
	argv[i++] = NULL;
	if (run_prg(argv))
		return -1;

	return 0;
}

static int drop_btrfs_subvol(struct mosaic *m, struct volume *t,
		int drop_flags)
{
	char *argv[8];
	char vol[PATH_MAX];
	int i;

	/*
	 * FIXME: locate subvolumes in subdirectories
	 */
	i = 0;
	argv[i++] = "btrfs";
	argv[i++] = "subvolume";
	argv[i++] = "delete";
	snprintf(vol, sizeof(vol), "%s/%s", m->m_loc, t->t_name);
	argv[i++] = vol;
	argv[i++] = NULL;
	if (run_prg(argv))
		return -1;

	return 0;
}

static int resize_btrfs_subvol(struct mosaic *m, struct volume *t,
		unsigned long size_in_blocks, int resize_flags)
{
	/* FIXME: qgroups */
	return -1;
}

static int get_btrfs_subvol_size(struct mosaic *m, struct volume *t,
		unsigned long *size_in_blocks)
{
	/* FIXME: qgroups */
	return -1;
}

const struct mosaic_ops mosaic_btrfs = {
	.name = "btrfs",

	.open = open_btrfs,

	.new_volume = new_btrfs_subvol,
	.open_volume = open_btrfs_subvol,
	.clone_volume = clone_btrfs_subvol,
	.drop_volume = drop_btrfs_subvol,
	.mount_volume = bind_vol_loc,
	.resize_volume = resize_btrfs_subvol,
	.get_volume_size = get_btrfs_subvol_size,

	.parse_layout = parse_mosaic_subdir_layout,
};
