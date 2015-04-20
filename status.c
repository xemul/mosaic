#include <sys/mount.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include "mosaic.h"
#include "status.h"

#define STATUS_DIR	"mosaic.status"

static int st_check_dir(void)
{
	if (!access(STATUS_DIR"/active", X_OK))
		return 0;

	if (mount("mosaic.status", STATUS_DIR, "tmpfs", 0, NULL)) {
		perror("Can't mount status dir");
		return 1;
	}

	/*
	 * FIXME -- record in mountinfo should be enough
	 */

	if (creat(STATUS_DIR"/active", 0600)) {
		perror("Can't create status dir");
		umount(STATUS_DIR);
		return 1;
	}

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
	 * FIXME 2 -- several mounts
	 */

	sprintf(st_aux, STATUS_DIR "/m.%s", m->m_name);
	st = fopen(st_aux, "w");
	fprintf(st, "- %s\n", path);
	fclose(st);
}

char *st_get_mounted(struct mosaic *m)
{
	FILE *st;

	if (st_check_dir())
		return;

	/*
	 * FIXME -- not good to report static string back
	 */

	sprintf(st_aux, STATUS_DIR "/m.%s", m->m_name);
	st = fopen(st_aux, "r");
	if (!st)
		return "-";

	fgets(st_aux, sizeof(st_aux), st);
	fclose(st);

	if (st_aux[0] != '-' || st_aux[1] != ' ') {
		printf("Status for %s is screwed up\n", m->m_name);
		return "X";
	}

	return st_aux + 2;
}
