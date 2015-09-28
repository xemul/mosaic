#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mount.h>

#include "mosaic.h"
#include "tessera.h"
#include "util.h"

#include "ploop-internal.h"

const char *uuidvar = "uuid-for-children";

static int create_ploop(struct mosaic *m, const char *name,
		unsigned long size_in_blocks, int make_flags)
{
	char dir[PATH_MAX];
	char img[PATH_MAX];
	char fstype[32];
	char size[32];
	char *argv[8];
	int i;

	snprintf(dir, sizeof(dir), "%s/%s", m->m_loc, name);
	// Assuming m->m_loc exists
	if (mkdir(dir, 0700) < 0) {
		// errno can be EEXIST or something else,
		// but we can only return -1 here
		return -1;
	}

	if (make_flags & NEW_TESS_WITH_FS) {
		if (m->default_fs)
			snprintf(fstype, sizeof(fstype),
					"-t %s", m->default_fs);
		else
			fstype[0] = '\0'; // default fs
	} else {
		strcpy(fstype, "-t none"); // no fs
	}

	i = 0;
	argv[i++] = "ploop";
	argv[i++] = "init";
	argv[i++] = "-s";
	snprintf(size, sizeof(size), "%lu", size_in_blocks);
	argv[i++] = size;
	argv[i++] = "-f";
	argv[i++] = "expanded";
	snprintf(img, sizeof(img), "%s/%s", dir, IMG_NAME);
	argv[i++] = img;
	argv[i++] = NULL;
	if (run_prg(argv) != 0)
		return -1;

	return 0;
}

static int open_ploop(struct mosaic *m, struct tessera *t, int open_flags)
{
	char dd[PATH_MAX];
	(void)open_flags; // unused

	/* Just check that corresponding dd.xml exists */
	snprintf(dd, sizeof(dd), "%s/%s/" DDXML, m->m_loc, t->t_name);
	if (access(dd, R_OK) < 0)
		return -1;

	/* FIXME: what is this function supposed to do?
	 * Do we need to mount ploop here /dev/ploopXXX is available?
	 * If yes, why there's no matching close_tessera()?
	 * Also, not all places calling this function need a device.
	 */

	return 0;
}

static int clone_ploop(struct mosaic *m, struct tessera *parent,
		const char *name, int clone_flags)
{
	/* While ploop is a set of layered images and it's easy to implement
	 * cloning mechanism, it is not (yet?) done in either library
	 * or the command line tool, so we have to dance around here.
	 *
	 * The idea is simple:
	 *  1. create a snapshot in "parent" ploop
	 *  2. copy dd.xml from parent dir
	 *  3. hardlink all images from parent dir
	 *  4. switch to that snapshot, removing old top delta
	 *
	 * Now, the parent snapshot we create at step 1 can be reused later for
	 * more clones (reducing number of snapshots as well as saving time),
	 * so we check first if we have such a snapshot already created.
	 * Once the parent is opened read-write, we can no longer use that
	 * snapshot as it's not up-to-date.
	 */
	char pdir[PATH_MAX], dir[PATH_MAX];
	char dd[PATH_MAX], pdd[PATH_MAX];
	int pdfd, dfd;
	char *argv[8];
	int i;
	(void)clone_flags; // unused

	snprintf(pdir, sizeof(pdir), "%s/%s", m->m_loc, parent->t_name);
	snprintf(pdd, sizeof(pdd), "%s/%s", pdir, DDXML);
	if (access(pdd, R_OK) < 0) {
		/* parent doesn't exist! */
		return -1;
	}

	snprintf(dir, sizeof(dir), "%s/%s", m->m_loc, name);
	snprintf(dd, sizeof(dd), "%s/%s", dir, DDXML);
	if (access(dd, R_OK) == 0) {
		/* name already exist! */
		return -1;
	}

	if (mkdir(dir, 0700) < 0) {
		fprintf(stderr, "%s: mkdir %s failed: %m\n", __func__, dir);
		return -1;
	}

	pdfd = open(pdir, O_DIRECTORY);
	if (pdfd < 0) {
		fprintf(stderr, "%s: can't open %s: %m\n", __func__, pdir);
		return -1;
	}

	dfd = open(dir, O_DIRECTORY);
	if (dfd < 0) {
		fprintf(stderr, "%s: can't open %s: %m\n", __func__, dir);
		return -1;
	}

	/* FIXME: lock parent ploop. HOW? */

	int ret = -1;
	/* 1. Check if we have parent already snapshotted for clone */
	char *uuid = read_var(pdfd, pdir, uuidvar);
	if (uuid == NULL) {

		/* 1.1 Create a new snapshot in parent */
		uuid = malloc(UUID_SIZE);
		if (ploop_uuid_generate(uuid, UUID_SIZE) != 0) {
			fprintf(stderr, "%s: can't generate uuid\n", __func__);
			goto out;
		}

		i = 0;
		argv[i++] = "ploop";
		argv[i++] = "snapshot";
		argv[i++] = "-u";
		argv[i++] = uuid;
		argv[i++] = pdd;
		argv[i++] = NULL;
		if (run_prg(argv) != 0) {
			// error is printed by ploop
			goto out;
		}
		/* Save for reuse */
		write_var(pdfd, pdir, uuidvar, uuid);
	}

	/* 2. Copy dd.xml from parent */
	if (copy_file(pdfd, pdir, dfd, dir, DDXML) != 0) {
		goto out;
	}

	/* 3. Hardlink all the deltas */
	DIR *d = fdopendir(pdfd);
	if (!d) {
		fprintf(stderr, "%s: opendir %s failed: %m\n", __func__, pdir);
		goto out;
	}
	struct dirent *de;
	while ((de = readdir(d))) {
		const char *name = de->d_name;

		// delta file names should start with IMG_NAME
		if (strncmp(name, IMG_NAME, sizeof(IMG_NAME) - 1) != 0)
			continue;
		if (linkat(pdfd, name, dfd, name, 0) != 0) {
			fprintf(stderr, "%s: can't hardlink "
					"%s/%s -> %s/%s: %m\n",
					__func__,
					dir, name, pdir, name);
			closedir(d);
			goto out;
		}

	}
	closedir(d);

	/* 4. Switch to created snapshot, removing old top delta
	 * and creating a new empty one instead.
	 */
	i = 0;
	argv[i++] = "ploop";
	argv[i++] = "snapshot-switch";
	argv[i++] = "-u";
	argv[i++] = uuid;
	argv[i++] = dd;
	argv[i++] = NULL;
	if (run_prg(argv) != 0) {
		goto out;
	}


	ret = 0;
out:
	if (ret != 0) {
		/* rollback: remove what we created */
		if (remove_rec(dfd) == 0)
			rmdir(dir);
	}

	free(uuid);
	close(pdfd);
	close(dfd);

	return ret;
}

static int mount_ploop(struct mosaic *m, struct tessera *t,
		const char *path, int mount_flags)
{
	char dd[PATH_MAX];
	char *argv[8];
	int ro = (mount_flags & MS_RDONLY);
	int i;

	snprintf(dd, sizeof(dd), "%s/%s/" DDXML, m->m_loc, t->t_name);

	i = 0;
	argv[i++] = "ploop";
	argv[i++] = "mount";
	argv[i++] = "-m";
	argv[i++] = (char *)path;
	if (ro)
		argv[i++] = "-r";
	argv[i++] = dd;
	argv[i++] = NULL;

	if (run_prg(argv) != 0)
		return -1;

	if (!ro) {
		char *slash;

		// make a directory out of 'dd' by removing file component
		slash = strrchr(dd, '/');
		*slash = '\0';
		// invalidate the snapshot used for cloning
		unset_var(-1, dd, uuidvar);
	}

	return 0;
}

static int umount_ploop(struct mosaic *m, struct tessera *t,
		const char *path, int umount_flags)
{
	char dd[PATH_MAX];
	char *argv[4];
	int i;
	(void)umount_flags; // unused
	(void)path; // unused

	snprintf(dd, sizeof(dd), "%s/%s/" DDXML, m->m_loc, t->t_name);

	/* FIXME: we can either umount by path, ddxml, or device.
	 * Not sure which one is the best way, let's settle for ddxml for now.
	 */
	i = 0;
	argv[i++] = "ploop";
	argv[i++] = "umount";
	argv[i++] = dd;
	argv[i++] = NULL;

	if (run_prg(argv) != 0)
		return -1;

	return 0;
}

static int remove_ploop(struct mosaic *m, struct tessera *t,
		int remove_flags)
{
	char dir[PATH_MAX];
	int dfd, ret;
	(void)remove_flags; // unused

	// FIXME: upper level should make sure we're not mounted!
	snprintf(dir, sizeof(dir), "%s/%s", m->m_loc, t->t_name);
	dfd = open(dir, O_DIRECTORY);
	if (dfd < 0) {
		fprintf(stderr, "%s: can't open %s: %m\n", __func__, dir);
		return -1;
	}
	ret = remove_rec(dfd);
	close(dfd);
	if (ret == 0)
		if (rmdir(dir))
			ret = -1;

	return ret;
}

static int resize_ploop(struct mosaic *m, struct tessera *t,
		unsigned long size_in_blocks, int resize_flags)
{
	char dd[PATH_MAX];
	char size[32];
	char *argv[8];
	int i;
	(void)resize_flags; // unused

	snprintf(dd, sizeof(dd), "%s/%s/" DDXML, m->m_loc, t->t_name);
	snprintf(size, sizeof(size), "%lu", size_in_blocks);

	i = 0;
	argv[i++] = "ploop";
	argv[i++] = "resize";
	argv[i++] = "-s";
	argv[i++] = size;
	argv[i++] = dd;
	argv[i++] = NULL;

	if (run_prg(argv) != 0)
		return -1;

	return 0;
}

static int get_size_ploop(struct mosaic *m, struct tessera *t,
		unsigned long *size_in_blocks)
{
	/* FIXME: we have total, used, and free for blocks and inodes,
	 * similar to what "df; df -i" shows. I.e. there are 6 values.
	 */
	return -1;
}

static int detach_ploop(struct mosaic *m, struct tessera *t, char *dev)
{
	(void)dev; // unused

	/* FIXME: if a filesystem within this ploop device
	 * is mounted, it is automatically unmounted.
	 * Ideally, we should return say EBUSY in this case.
	 */
	return umount_ploop(m, t, NULL, 0);
}

static int attach_ploop(struct mosaic *m, struct tessera *t,
		char *dev, int size, int flags)
{
	const char *needle = "Adding delta dev=/dev/ploop";
	const int dev_offset = sizeof("Adding delta dev=");
	char cmd[PATH_MAX];
	char dd[PATH_MAX];
	char out[PATH_MAX];
	FILE *fp;
	int ret = -1, rc;
	(void)flags; // unused

	/* Theoretically this is the same as mount_ploop() only
	 * without -m argument, but we need to figure out device
	 * name and the only way possible when using the command
	 * line tool is to parse the command output string,
	 * looking for a specific string.
	 */

	snprintf(dd, sizeof(dd), "%s/%s/" DDXML, m->m_loc, t->t_name);
	snprintf(cmd, sizeof(cmd), "ploop mount %s", dd);

	dev[0] = '\0'; // assume len > 0
	fp = popen(cmd, "re");
	if (!fp) {
		fprintf(stderr, "%s: can't popen %s: %m\n", __func__, cmd);
		return -1;
	}

	/* Parse the output to find the device name */
	while (fgets(out, sizeof(out), fp) != NULL) {
		char *str, *begin, *end;
		if (ret > 0) // already found
			continue;
		str = strstr(out, needle);
		if (!str)
			continue;
		begin = str + dev_offset - 1;
		end = strchr(begin, ' ');
		if (!end)
			continue; // hmm, read on a boundary?
		*end = '\0';
		if (end - begin + 1 > size)
			fprintf(stderr, "%s: not enough buffer space (%d)"
					"to store %s",
					__func__, size, begin);
		strcpy(dev, begin);
		ret = end - begin;
	}

	/* Error handling is a bit cumbersome below and cries for a comment.
	 *
	 * First, we check that plose did not return an error.
	 * Second, we check that ploop exit code is not zero.
	 * Finally, we check that we got the device name.
	 *
	 * Note that the ret = -1 assignments below are probably redundant,
	 * as in case of ploop error we probably haven't got a device
	 * string so ret is -1 anyway, but better be safe than sorry.
	 */
	rc = pclose(fp);
	if (rc < 0) {
		fprintf(stderr, "%s: command execution error, pclose(): %m\n",
				__func__);
		ret = -1;
	} else if (rc) {
		fprintf(stderr, "%s: ploop returned non-zero exit code %d\n",
				__func__, WEXITSTATUS(rc));
		ret = -1;
	} else if (ret == -1) {
		/* The ploop command succeeded, i.e. device is mounted,
		 * but we were unable to parse the device name string
		 * from the command output, so need to rollback.
		 */
		detach_ploop(m, t, NULL);
		fprintf(stderr, "%s: internal error: can't parse dev\n",
				__func__);
	}

	return ret;
}

const struct mosaic_ops mosaic_ploop = {
	.name		= "ploop",

	/* ploop ops */
	.new_tessera	= create_ploop,
	.open_tessera	= open_ploop,
	.clone_tessera	= clone_ploop,
	.mount_tessera	= mount_ploop,
	.umount_tessera	= umount_ploop,
	.drop_tessera	= remove_ploop,
	.resize_tessera	= resize_ploop,
	.get_tessera_size = get_size_ploop,
	.attach_tessera = attach_ploop,
	.detach_tessera = detach_ploop,
};
