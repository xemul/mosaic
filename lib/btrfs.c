#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

	// Make sure upper directories exist
	snprintf(vol, sizeof(vol), "%s/%s", m->m_loc, name);
	if (mkdir_p(vol, 0, 0700) < 0) {
		// error is printed by mkdir_p()
		return -1;
	}

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

static int have_btrfs_subvol(struct mosaic *m, const char *name, int flags)
{
	char vol[PATH_MAX];
	struct stat st;
	(void)flags; // unused

	snprintf(vol, sizeof(vol), "%s/%s", m->m_loc, name);
	if (stat(vol, &st) == 0) {
		if S_ISDIR(st.st_mode)
			return 1; // exists

		loge("%s: %s exists but is not a btrfs subvolume\n",
				__func__, name);
		return -1; // error
	}
	if (errno == ENOENT)
		return 0; // does not exist

	loge("%s: stat(%s) failed: %m\n", __func__, vol);
	return -1; // error
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

	// Make sure upper directories exist
	snprintf(vol, sizeof(vol), "%s/%s", m->m_loc, name);
	if (mkdir_p(vol, 0, 0700) < 0) {
		// error is printed by mkdir_p()
		return -1;
	}

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
	const char *base = m->m_loc;
	int i, ret, dfd;

	/*
	 * FIXME: locate subvolumes in subdirectories
	 */
	i = 0;
	argv[i++] = "btrfs";
	argv[i++] = "subvolume";
	argv[i++] = "delete";
	snprintf(vol, sizeof(vol), "%s/%s", base, t->t_name);
	argv[i++] = vol;
	argv[i++] = NULL;
	if (run_prg(argv))
		return -1;

	// Remove all the non-empty parent directories up to base
	dfd = open(base, O_DIRECTORY);
	if (dfd < 0) {
		loge("%s: can't open %s: %m\n", __func__, base);
		return -1;
	}
	ret = rmdirat_r(dfd, base, t->t_name);
	close(dfd);

	return ret;
}

/* FIXME: qgroups
static int resize_btrfs_subvol(struct mosaic *m, struct volume *t,
		unsigned long size_in_blocks, int resize_flags)
{
	return -1;
}
*/

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
	.have_volume = have_btrfs_subvol,
	.clone_volume = clone_btrfs_subvol,
	.drop_volume = drop_btrfs_subvol,
	.mount_volume = bind_vol_loc,
	.get_volume_size = get_btrfs_subvol_size,

	.parse_layout = parse_mosaic_subdir_layout,
};
