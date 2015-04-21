#include <sys/mount.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "mosaic.h"
#include "status.h"

#define STATUS_DIR	"mosaic.status"

static int st_check_dir(void)
{
	int fd;

	if (!access(STATUS_DIR"/active", F_OK))
		return 0;

	if (mount("mosaic.status", STATUS_DIR, "tmpfs", 0, NULL)) {
		perror("Can't mount status dir");
		return 1;
	}

	/*
	 * FIXME -- record in mountinfo should be enough
	 */

	fd = creat(STATUS_DIR"/active", 0600);
	if (fd < 0) {
		perror("Can't create status dir");
		umount(STATUS_DIR);
		return 1;
	}

	close(fd);
	return 0;
}

static char st_aux[PATH_MAX];

void st_set_mounted(struct mosaic *m, char *path)
{
	FILE *st;

	if (st_check_dir())
		return;

	/*
	 * FIXME -- this is YAML, but it's handled manually
	 * FIXME -- locking
	 */

	sprintf(st_aux, STATUS_DIR "/m.%s", m->m_name);
	st = fopen(st_aux, "a+");
	if (!st)
		goto skip;

	while (fgets(st_aux, sizeof(st_aux), st)) {
		if (st_aux[0] != '-' || st_aux[1] != ' ') {
			/* BUG */
			printf("Fatal. State file screwed up.\n");
			goto done;
		}

		if (!strcmp(st_aux + 2, path))
			/* already there */
			goto done;
	}

skip:
	fprintf(st, "- %s\n", path);
done:
	fclose(st);
}

static int st_for_each_mounted(struct mosaic *m, int (*cb)(struct mosaic *, char *, void *), void *x)
{
	FILE *st;
	int ret = 0;
	char *e;

	if (st_check_dir())
		return -1;

	sprintf(st_aux, STATUS_DIR "/m.%s", m->m_name);
	st = fopen(st_aux, "r");
	if (!st)
		return -1;

	while (fgets(st_aux, sizeof(st_aux), st)) {
		if (st_aux[0] != '-' || st_aux[1] != ' ') {
			/* BUG */
			ret = -1;
			printf("Fatal. State file screwed up.\n");
			fclose(st);
			break;
		}

		e = strchr(st_aux, '\n');
		*e = '\0';

		ret = cb(m, st_aux + 2, x);
		if (ret != 0)
			break;
	}

	fclose(st);
	return ret;
}

struct st_umount_ctx {
	char *path;
	int (*cb)(struct mosaic *, char *p);
	FILE *nst;
};

static int umount_one(struct mosaic *m, char *path, void *x)
{
	struct st_umount_ctx *uc = x;

	if (uc->path && strcmp(uc->path, path)) {
		fprintf(uc->nst, "- %s\n", path);
		return 0;
	}

	if (uc->cb(m, path))
		return -1;

	return 0;
}

int st_umount(struct mosaic *m, char *path, int (*cb)(struct mosaic *, char *p))
{
	struct st_umount_ctx uc;
	FILE *nst;
	int ret;

	sprintf(st_aux, STATUS_DIR "/m.%s.upd", m->m_name);
	nst = fopen(st_aux, "w");
	if (!nst)
		return -1;

	uc.path = path;
	uc.cb = cb;
	uc.nst = nst;
	ret = st_for_each_mounted(m, umount_one, &uc);

	fclose(nst);
	sprintf(st_aux, STATUS_DIR "/m.%s.upd", m->m_name);

	if (!ret) {
		char aux2[PATH_MAX];
		sprintf(aux2, STATUS_DIR "/m.%s", m->m_name);
		ret = rename(st_aux, aux2);
	} else
		unlink(st_aux);

	return ret;
}

int mosaic_iterate_mounted(struct mosaic *m, int (*cb)(struct mosaic *, char *, void *), void *x)
{
	return st_for_each_mounted(m, cb, x);
}
