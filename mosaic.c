#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mosaic.h"
#include "tessera.h"
#include "util.h"
#include "status.h"

struct mosaic *mosaic_find_by_name(char *name)
{
	struct mosaic *m;

	list_for_each_entry(m, &ms->mosaics, sl)
		if (!strcmp(m->m_name, name))
			return m;

	return NULL;
}

static int do_umount_mosaic_from(struct mosaic *m, char *mp, void *x)
{
	char *chk = x;
	struct element *e;
	char path[PATH_MAX];
	int plen;

	if (chk && strcmp(mp, chk))
		return ST_OK;

	plen = sprintf(path, "%s/", mp);
	list_for_each_entry(e, &m->elements, ml) {
		sprintf(path + plen, "%s", e->e_at);
		if (umount(path)) {
			perror("Can't umount");
			return ST_FAIL;
		}
	}

	return ST_DROP;
}


int mosaic_mount(struct mosaic *m, char *mp_path, char *options)
{
	struct stat buf;
	struct element *el;
	char path[PATH_MAX];
	int plen;

	if (options) {
		printf("Mount options not yet supported\n");
		return -1;
	}

	if (stat(mp_path, &buf)) {
		printf("Can't stat %s\n", mp_path);
		return -1;
	}

	if (!S_ISDIR(buf.st_mode)) {
		printf("Can't mount mosaic on non-directory\n");
		return -1;
	}

	plen = sprintf(path, "%s/", mp_path);

	list_for_each_entry(el, &m->elements, ml) {
		sprintf(path + plen, "%s", el->e_at);
		if (do_mount_tessera_at(el->t, el->e_age, path, el->e_options))
			goto umount;
	}

	st_set_mounted(m, mp_path);

	return 0;

umount:
	do_umount_mosaic_from(m, mp_path, NULL);
	return -1;
}

int mosaic_umount(struct mosaic *m, char *from)
{
	return st_for_each_mounted(m, true, do_umount_mosaic_from, from);
}

int mosaic_iterate(int (*cb)(struct mosaic *m, void *x), void *x)
{
	struct mosaic *m;
	int ret = 0;

	list_for_each_entry(m, &ms->mosaics, sl)
		if (ret = cb(m, x))
			break;

	return ret;
}

int mosaic_iterate_elements(struct mosaic *m, int (*cb)(struct mosaic *, struct element *, void *), void *x)
{
	struct element *e;
	int ret = 0;

	list_for_each_entry(e, &m->elements, ml)
		if (ret = cb(m, e, x))
			break;

	return ret;
}

struct mosaic *mosaic_new(char *name)
{
	struct mosaic *m;

	m = malloc(sizeof(*m));
	if (m) {
		m->m_name = strdup(name);
		INIT_LIST_HEAD(&m->elements);

	}

	return m;
}

static struct element *find_element(struct mosaic *m, char *name)
{
	struct element *e;

	list_for_each_entry(e, &m->elements, ml)
		if (!strcmp(e->t->t_name, name))
			return e;

	return NULL;
}

int mosaic_set_element(struct mosaic *m, char *name, int age, char *at, char *opt)
{
	struct element *e;

	e = find_element(m, name);
	if (e) {
		free(e->e_at);
		free(e->e_options);
	} else {
		struct tessera *t;

		t = find_tessera(ms, name);
		if (!t)
			return -1;

		e = malloc(sizeof(*e));
		e->t = t;
	}

found:
	e->e_age = age;
	e->e_at = strdup(at);
	e->e_options = NULL;

	if (opt) {
		char *a;

		a = strchr(opt, '=');
		if (!a || strncmp(opt, "options", a - opt)) {
			free(e->e_at);
			free(e);
			return -1;
		}

		e->e_options = strdup(a + 1);
	}

	list_add_tail(&e->ml, &m->elements);
	return 0;
}

int mosaic_del_element(struct mosaic *m, char *name)
{
	struct element *e;

	e = find_element(m, name);
	if (!e)
		return -1;

	list_del(&e->ml);
	free(e->e_at);
	free(e->e_options);
	free(e);

	return 0;
}

int mosaic_add(struct mosaic *m)
{
	list_add_tail(&m->sl, &ms->mosaics);
	return config_update();
}

int mosaic_update(struct mosaic *m)
{
	return config_update();
}

int mosaic_del(struct mosaic *m)
{
	struct element *e, *n;

	/* FIXME -- what if mounted? */

	list_for_each_entry_safe(e, n, &m->elements, ml) {
		free(e->e_options);
		free(e->e_at);
		free(e);
	}

	list_del(&m->sl);
	free(m);

	return config_update();
}
