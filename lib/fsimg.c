#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include "mosaic.h"
#include "volume.h"
#include "util.h"

static int open_fsimg(struct mosaic *m, int flags)
{
	if (!m->default_fs)
		m->default_fs = strdup("ext4");

	return open_mosaic_subdir(m);
}


static int open_fsimg_vol(struct mosaic *m, struct volume *t,
		int open_flags)
{
	struct mosaic_subdir_priv *fp = m->priv;
	struct stat b;

	return fstatat(fp->m_loc_dir, t->t_name, &b, 0);
}

static int new_fsimg_vol(struct mosaic *m, const char *name,
		unsigned long size_in_blocks, int make_flags)
{
	struct mosaic_subdir_priv *fp = m->priv;
	char dir[PATH_MAX];
	int imgf;

	/*
	 * FIXME -- add separate subdir for images and separate for
	 * regular fs mount?
	 */
	snprintf(dir, sizeof(dir), "%s/%s", m->m_loc, name);
	if (mkdir_p(dir, 0, 0700) < 0) {
		// error is printed by mkdir_p()
		return -1;
	}
	imgf = openat(fp->m_loc_dir, name, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (imgf < 0) {
		fprintf(stderr, "%s: can't create %s: %m\n", __func__, name);
		return -1;
	}

	if (ftruncate(imgf, size_in_blocks << MOSAIC_BLOCK_SHIFT) < 0) {
		fprintf(stderr, "%s: can't grow %s: %m\n", __func__, name);
		close(imgf);
		return -1;
	}

	if (make_flags & NEW_VOL_WITH_FS) {
		char path[PATH_MAX];
		char *argv[8];
		int i = 0;

		snprintf(path, sizeof(path), "%s/%s", m->m_loc, name);

		argv[i++] = "mkfs";
		argv[i++] = "-t";
		argv[i++] = m->default_fs;
		argv[i++] = "-F";
		argv[i++] = path;
		argv[i++] = NULL;
		if (run_prg(argv)) {
			close(imgf);
			return -1;
		}
	}

	close(imgf);
	return 0;
}

static int drop_fsimg_vol(struct mosaic *m, struct volume *t,
		int drop_flags)
{
	const char *base = m->m_loc;
	struct mosaic_subdir_priv *fp = m->priv;
	int base_fd = fp->m_loc_dir;

	/*
	 * FIXME -- what if mounted?
	 */
	if (unlinkat(base_fd, t->t_name, 0)) {
		fprintf(stderr, "%s: can't rm %s: %m\n", __func__, t->t_name);
		return -1;
	}
	// Remove all the non-empty parent directories up to base
	return rmdirat_r(base_fd, base, t->t_name);
}

/* Figure out device name that corresponds to given volume.
 * Returns device string length, 0 if no device, or -1 on error.
 */
static int get_dev(struct mosaic *m, struct volume *t, char *dev, int size)
{
	struct mosaic_subdir_priv *pdata = m->priv;
	char cmd[1024];
	char *line = NULL;
	size_t llen = 0;
	int lcnt = 0;
	char *end;
	FILE *fp;
	int ret = -1;
	int rc;

	// sanity check first
	if (!pdata || pdata ->m_loc_dir < 0) {
		fprintf(stderr, "%s: internal error, m_loc_dir unset\n",
				__func__);
		return -1;
	}

	snprintf(cmd, sizeof(cmd), "losetup --associated /proc/self/fd/%d/%s",
			pdata->m_loc_dir, t->t_name);

	dev[0] = '\0';
	fp = popen(cmd, "re");
	if (!fp) {
		fprintf(stderr, "%s: can't run %s: %m\n", __func__, cmd);
		return -1;
	}

	/* There can be many lines of output, we need to use the last one
	 * as in case of multiple devices we need to get the most recent.
	 */
	errno = 0;
	while ((rc = getline(&line, &llen, fp)) != -1) {
		lcnt++;
		// we are only interested in the last line, skip the rest
	}
	if (rc == -1 && errno != 0) {
		fprintf(stderr, "%s: error reading from %s: %m\n",
				__func__, cmd);
		goto out;
	}
	if (lcnt == 0) { // no output
		ret = 0; // no device
		goto out;
	}

	/* Output should look like this:
	 * /dev/loop2: [fc01]:1184495 (/path/to/image)
	 * Make sure to check it carefully.
	 */
	end = strchr(line, ':');
	if (strncmp(line, "/dev/", 5) != 0 || !end) {
		fprintf(stderr, "%s: unexpected losetup output: \"%s\"\n",
				__func__, line);
		goto out;
	}

	*end = '\0';
	if (end - line + 1 > size) {
		fprintf(stderr, "%s: not enough buffer space (%d)"
				" to store %s\n", __func__, size, line);
		goto out;
	}
	strcpy(dev, line);
	ret = end - line;

out:
	rc = pclose(fp);
	/* We succeed only if
	 *  1 pclose() did not return an error
	 *  2 losetup exit code is zero
	 *
	 * Note that ret = -1 assignments below are probably redundant.
	 */
	if (rc < 0) {
		fprintf(stderr, "%s: command execution error, pclose(): %m\n",
				__func__);
		ret = -1;
	} else if (rc) {
		fprintf(stderr, "%s: ploop returned non-zero exit code %d\n",
				__func__, WEXITSTATUS(rc));
		ret = -1;
	}
	free(line);

	return ret;
}

static int attach_fsimg_vol(struct mosaic *m, struct volume *t,
		char *dev, int len, int flags)
{
	struct mosaic_subdir_priv *fp = m->priv;
	char aux[1024], *nl;
	FILE *lsp;

	// First, check if the volume is already attached
	if (get_dev(m, t, aux, sizeof(aux)) > 0) {
		fprintf(stderr, "%s: %s already attached to %s\n",
				__func__, t->t_name, aux);
		return -1;
	}

	/*
	 * FIXME: call losetup by hands?
	 */
	sprintf(aux, "losetup --find --show /proc/self/fd/%d/%s", fp->m_loc_dir, t->t_name);
	lsp = popen(aux, "r");
	if (!lsp)
		return -1;

	fgets(aux, sizeof(aux), lsp);
	pclose(lsp);

	nl = strchr(aux, '\n');
	if (nl)
		*nl = '\0';

	strncpy(dev, aux, len);
	return strlen(aux);
}

static int detach_fsimg_vol(struct mosaic *m, struct volume *t)
{
	char *argv[4];
	char dev[NAME_MAX];
	int i = 0;
	int rc;

	rc = get_dev(m, t, dev, sizeof(dev));
	if (rc < 0) { // error
		fprintf(stderr, "%s: error getting device\n", __func__);
		return -1;
	}
	if (rc == 0) {
		// no device
		fprintf(stderr, "%s: volume %s not attached\n",
				__func__, t->t_name);
		return -1;
	}

	argv[i++] = "losetup";
	argv[i++] = "-d";
	argv[i++] = dev;
	argv[i++] = NULL;

	if (run_prg(argv))
		return -1;

	return 0;
}

static int resize_fsimg_vol(struct mosaic *m, struct volume *t,
		unsigned long size_in_blocks, int resize_flags)
{
	/* FIXME */
	return -1;
}

static int get_fsimg_size(struct mosaic *m, struct volume *t, unsigned long *size_in_blocks)
{
	struct mosaic_subdir_priv *fp = m->priv;
	struct stat buf;

	if (fstatat(fp->m_loc_dir, t->t_name, &buf, 0))
		return -1;

	*size_in_blocks = buf.st_size >> MOSAIC_BLOCK_SHIFT;
	return 0;
}

const struct mosaic_ops mosaic_fsimg = {
	.name = "fsimg",
	.open = open_fsimg,
/*	.mount = FIXME: location can be device */

	.open_volume = open_fsimg_vol,
	.new_volume = new_fsimg_vol,
	.drop_volume = drop_fsimg_vol,
	.attach_volume = attach_fsimg_vol,
	.detach_volume = detach_fsimg_vol,
	.resize_volume = resize_fsimg_vol,
	.get_volume_size = get_fsimg_size,

	.parse_layout = parse_mosaic_subdir_layout,
};
