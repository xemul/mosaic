#include <sys/mount.h>
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

int st_for_each_mounted(struct mosaic *m, bool mod, int (*cb)(struct mosaic *, char *, void *), void *x)
{
	FILE *st, *nst = NULL;
	int ret;
	char *e;

	if (st_check_dir())
		return -1;

	sprintf(st_aux, STATUS_DIR "/m.%s", m->m_name);
	st = fopen(st_aux, "r");
	if (!st)
		return -1;

	if (mod) {
		sprintf(st_aux, STATUS_DIR "/m.%s.upd", m->m_name);
		nst = fopen(st_aux, "w");
		if (!nst) {
			fclose(st);
			return -1;
		}
	}

	while (fgets(st_aux, sizeof(st_aux), st)) {
		if (st_aux[0] != '-' || st_aux[1] != ' ') {
			/* BUG */
			printf("Fatal. State file screwed up.\n");
			fclose(st);
			break;
		}

		e = strchr(st_aux, '\n');
		*e = '\0';

		ret = cb(m, st_aux + 2, x);
		if (ret == ST_FAIL) {
			fclose(st);
			if (nst) {
				sprintf(st_aux, STATUS_DIR "/m.%s.upd", m->m_name);
				fclose(nst);
				unlink(st_aux);
			}

			return -1;
		}

		if (!nst || ret == ST_DROP)
			continue;

		if (ret == ST_DROP)
			continue;

		fprintf(nst, "- %s\n", st_aux + 2);
	}

	fclose(st);
	if (nst) {
		char aux2[PATH_MAX];

		fclose(nst);
		sprintf(st_aux, STATUS_DIR "/m.%s.upd", m->m_name);
		sprintf(aux2, STATUS_DIR "/m.%s", m->m_name);
		if (rename(st_aux, aux2))
			return -1;
	}

	return 0;
}

static int show_mounted(struct mosaic *m, char *path, void *_x)
{
	int *mnt = _x;

	if (!*mnt) {
		printf("mounted:\n");
		*mnt = 1;
	}

	printf("  - %s\n", path);
	return 0;
}

void st_show_mounted(struct mosaic *m)
{
	int mnt = 0;

	st_for_each_mounted(m, false, show_mounted, &mnt);

	if (mnt)
		printf("\n");
}
