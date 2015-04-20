#include <sys/mount.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
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

void st_show_mounted(struct mosaic *m)
{
	FILE *st;
	char *s;
	bool mnt = false;

	if (st_check_dir())
		return;

	/*
	 * FIXME -- not good to report static string back
	 */

	sprintf(st_aux, STATUS_DIR "/m.%s", m->m_name);
	st = fopen(st_aux, "r");
	if (!st)
		return;

	while (fgets(st_aux, sizeof(st_aux), st)) {
		if (st_aux[0] != '-' || st_aux[1] != ' ') {
			/* BUG */
			printf("Fatal. State file screwed up.\n");
			fclose(st);
			break;
		}

		if (!mnt) {
			printf("mounted:\n");
			mnt = true;
		}

		printf("  %s", st_aux);
	}

	if (mnt)
		printf("\n");

	fclose(st);
}
