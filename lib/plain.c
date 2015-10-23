#include <unistd.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include "mosaic.h"
#include "volume.h"
#include "util.h"

static int open_plain(struct mosaic *m, int open_flags)
{
	if (m->default_fs)
		return -1;

	return open_mosaic_subdir(m);
}


static int new_plain_vol(struct mosaic *m, const char *name,
		unsigned long size_in_blocks, int new_flags)
{
	char dir[PATH_MAX];

	if (!(new_flags & NEW_VOL_WITH_FS))
		return -1;

	/* FIXME -- size_in_blocks ignored */
	snprintf(dir, sizeof(dir), "%s/%s", m->m_loc, name);
	return mkdir_p(dir, 1, 0600);
}

static int have_plain_vol(struct mosaic *m, const char *name, int flags)
{
	struct mosaic_subdir_priv *pp = m->priv;
	struct stat st;
	(void)flags; // unused

	if (fstatat(pp->m_loc_dir, name, &st, 0) == 0) {
		if (S_ISDIR(st.st_mode))
			return 1; // exists

		loge("%s: %s exists but is not a directory\n",
				__func__, name);
		return -1;
	}

	if (errno == ENOENT)
		return 0;

	loge("%s: stat(%s) failed: %m\n", __func__, name);
	return -1;
}

static int drop_plain_vol(struct mosaic *m, struct volume *t,
		int drop_flags)
{
	const char *base = m->m_loc;
	struct mosaic_subdir_priv *fp = m->priv;
	int base_fd = fp->m_loc_dir;
	int fd;

	fd = openat(base_fd, t->t_name, O_DIRECTORY);
	if (fd < 0) {
		loge("%s: can't open %s/%s: %m\n",
				__func__, base, t->t_name);
		return -1;
	}

	if (remove_rec(fd))
		return -1;

	// Remove all the non-empty parent directories up to base
	return rmdirat_r(base_fd, base, t->t_name);
}

static int get_plain_size(struct mosaic *m, struct volume *t,
		unsigned long *size_in_blocks)
{
	struct mosaic_subdir_priv *pp = m->priv;
	int fd, ret;

	fd = openat(pp->m_loc_dir, t->t_name, O_DIRECTORY);
	if (fd < 0)
		return -1;

	 ret = get_subdir_size(fd, size_in_blocks);
	 close(fd);

	 *size_in_blocks >>= MOSAIC_BLOCK_SHIFT;

	 return ret;
}

const struct mosaic_ops mosaic_plain = {
	.name = "plain",

	.open = open_plain,

	.new_volume = new_plain_vol,
	.have_volume = have_plain_vol,
	.drop_volume = drop_plain_vol,
	.mount_volume = bind_vol_loc,
	.get_volume_size = get_plain_size,

	.parse_layout = parse_mosaic_subdir_layout,
};
