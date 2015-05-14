#include <sys/mount.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "mosaic.h"
#include "status.h"
#include "log.h"

/* Run-Time dir for state that disappears after reboot */
#define STATUS_RUN_DIR	STATUS_DIR "/rt"

/*
 * Generic "ID" level
 */

static int st_check_dir(void)
{
	int fd;

	if (!access(STATUS_RUN_DIR"/active", F_OK))
		return 0;

	if (mount("mosaic.status", STATUS_RUN_DIR, "tmpfs", 0, NULL)) {
		loge("Can't mount status dir");
		return 1;
	}

	fd = creat(STATUS_RUN_DIR"/active", 0600);
	if (fd < 0) {
		loge("Can't create status dir");
		umount(STATUS_RUN_DIR);
		return 1;
	}

	close(fd);
	return 0;
}

static char st_aux[PATH_MAX];

static void set_mounted(char *id, char *path)
{
	FILE *st;

	if (st_check_dir())
		return;

	/*
	 * FIXME -- this is YAML, but it's handled manually
	 * FIXME -- locking
	 */

	sprintf(st_aux, STATUS_RUN_DIR "/%s", id);
	st = fopen(st_aux, "a+");
	if (!st)
		goto skip;

	while (fgets(st_aux, sizeof(st_aux), st)) {
		if (st_aux[0] != '-' || st_aux[1] != ' ') {
			/* BUG */
			log("FATAL. State file screwed up.\n");
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

static int st_for_each_mounted(char *id, int (*cb)(char *, void *), void *x)
{
	FILE *st;
	int ret = 0;
	char *e;

	if (st_check_dir())
		return -1;

	sprintf(st_aux, STATUS_RUN_DIR "/%s", id);
	st = fopen(st_aux, "r");
	if (!st)
		return -1;

	while (fgets(st_aux, sizeof(st_aux), st)) {
		if (st_aux[0] != '-' || st_aux[1] != ' ') {
			/* BUG */
			ret = -1;
			log("FATAL. State file screwed up.\n");
			fclose(st);
			break;
		}

		e = strchr(st_aux, '\n');
		*e = '\0';

		ret = cb(st_aux + 2, x);
		if (ret != 0)
			break;
	}

	fclose(st);
	return ret;
}

struct umount_ctx {
	char *path;
	FILE *nst;

	int (*cb)(char *, void *x);
	void *cb_arg;
	int empty;
};

static int umount_one(char *path, void *x)
{
	struct umount_ctx *uc = x;

	if (uc->path && strcmp(uc->path, path)) {
		uc->empty = 0;
		fprintf(uc->nst, "- %s\n", path);
		return 0;
	}

	if (uc->cb(path, uc->cb_arg))
		return -1;

	return 0;
}

static int do_umount(char *id, char *path, int (*cb)(char *, void *), void *x)
{
	struct umount_ctx uc;
	FILE *nst;
	int ret;

	sprintf(st_aux, STATUS_RUN_DIR "/.%s.upd", id);
	nst = fopen(st_aux, "w");
	if (!nst)
		return -1;

	uc.path = path;
	uc.nst = nst;
	uc.cb = cb;
	uc.cb_arg = x;
	uc.empty = 1;

	ret = st_for_each_mounted(id, umount_one, &uc);

	fclose(nst);
	sprintf(st_aux, STATUS_RUN_DIR "/.%s.upd", id);

	if (!ret) {
		char aux2[PATH_MAX];

		sprintf(aux2, STATUS_RUN_DIR "/%s", id);

		if (uc.empty) {
			unlink(st_aux);
			ret = unlink(aux2);
		} else
			ret = rename(st_aux, aux2);
	} else
		unlink(st_aux);

	return ret;
}

/*
 * Mosaic level
 */

void st_set_mounted(struct mosaic *m, char *path)
{
	char id[512];

	sprintf(id, "mos.%s", m->m_name);
	set_mounted(id, path);
}

int st_is_mounted(struct mosaic *m)
{
	char path[PATH_MAX];

	sprintf(path, STATUS_RUN_DIR "/mos.%s", m->m_name);
	return access(path, F_OK) == 0 ? 1 : 0;
}

struct m_iter_ctx {
	struct mosaic *m;
	union {
		int (*cb_x)(struct mosaic *, char *, void *);
		int (*cb)(struct mosaic *, char *);
	};
	void *cb_arg;
};

static int mosaic_iter_one(char *path, void *x)
{
	struct m_iter_ctx *i = x;
	return i->cb_x(i->m, path, i->cb_arg);
}

int mosaic_iterate_mounted(struct mosaic *m, int (*cb)(struct mosaic *, char *, void *), void *x)
{
	char id[512];
	struct m_iter_ctx i;

	i.m = m;
	i.cb_x = cb;
	i.cb_arg = x;

	sprintf(id, "mos.%s", m->m_name);
	return st_for_each_mounted(id, mosaic_iter_one, &i);
}

static int umount_mosaic(char *path, void *x)
{
	struct m_iter_ctx *i = x;
	return i->cb(i->m, path);
}

int st_umount(struct mosaic *m, char *from, int (*cb)(struct mosaic *, char *p))
{
	char id[512];
	struct m_iter_ctx i;

	i.m = m;
	i.cb = cb;

	sprintf(id, "mos.%s", m->m_name);
	return do_umount(id, from, umount_mosaic, &i);
}

/*
 * Tessera level
 */

void st_set_mounted_t(struct tessera *t, int age, char *path)
{
	char id[512];

	sprintf(id, "tes.%s.%d", t->t_name, age);
	set_mounted(id, path);
}

int st_is_mounted_t(struct tessera *t, int age, void *x)
{
	char path[PATH_MAX];

	sprintf(path, STATUS_RUN_DIR "/tes.%s.%d", t->t_name, age);
	return access(path, F_OK) == 0 ? 1 : 0;
}

struct t_iter_ctx {
	struct tessera *t;
	int age;
	union {
		int (*cb_x)(struct tessera *, int, char *, void *);
		int (*cb)(struct tessera *, int, char *);
	};
	void *cb_arg;
};

static int tessera_iter_one(char *path, void *x)
{
	struct t_iter_ctx *i = x;
	return i->cb_x(i->t, i->age, path, i->cb_arg);
}

int mosaic_iterate_mounted_t(struct tessera *t, int age, int (*cb)(struct tessera *, int age, char *, void *), void *x)
{
	char id[512];
	struct t_iter_ctx i;

	i.t = t;
	i.age = age;
	i.cb_x = cb;
	i.cb_arg = x;

	sprintf(id, "tes.%s.%d", t->t_name, age);
	return st_for_each_mounted(id, tessera_iter_one, &i);
}

static int umount_tessera(char *path, void *x)
{
	struct t_iter_ctx *i = x;
	return i->cb(i->t, i->age, path);
}

int st_umount_t(struct tessera *t, int age, char *from, int (*cb)(struct tessera *, int, char *))
{
	char id[512];
	struct t_iter_ctx i;

	i.t = t;
	i.age = age;
	i.cb = cb;

	sprintf(id, "tes.%s.%d", t->t_name, age);
	return do_umount(id, from, umount_tessera, &i);
}
