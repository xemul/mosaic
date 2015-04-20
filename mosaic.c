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

static struct mosaic *find_mosaic(char *name)
{
	struct mosaic *m;

	list_for_each_entry(m, &ms->mosaics, sl)
		if (!strcmp(m->m_name, name))
			return m;

	return NULL;
}

static int show_mosaic(int argc, char **argv)
{
	struct mosaic *m;
	struct element *e;

	if (argc < 1) {
		printf("Usage: moctl mosaic show <name>\n");
		return 1;
	}

	m = find_mosaic(argv[0]);
	if (!m) {
		printf("No such mosaic %s\n", argv[0]);
		return 1;
	}

	st_show_mounted(m);

	if (!list_empty(&m->elements))
		printf("elements:\n");

	list_for_each_entry(e, &m->elements, ml) {
		printf("  - name: %s\n", e->t->t_name);
		if (e->e_age == 0)
			printf("    age:     base\n");
		else
			printf("    age:     %d\n", e->e_age);
		printf("    at:      %s\n", e->e_at ? : "-");
		printf("    options: %s\n", e->e_options ? : "none");
		printf("\n");
	}
}

static int list_mosaics(void)
{
	struct mosaic *m;

	list_for_each_entry(m, &ms->mosaics, sl)
		printf("%s\n", m->m_name);

	return 0;
}

/*
 * Element description is
 *
 *  name:age:at[:options=$opts]
 *
 * Name is trimmed before calling set_element()
 */

static int set_element(struct element *e, char *desc)
{
	char *aux;

	/*
	 * Age
	 */
	aux = strchr(desc, ':');
	if (!aux) {
		printf("Missing parameter in element\n");
		return -1;
	}

	*aux = '\0';
	e->e_age = atoi(desc);

	/*
	 * Location
	 */
	desc = aux + 1;
	aux = strchr(desc, ':');
	if (aux)
		*aux = '\0';

	e->e_at = desc;

	if (!aux)
		return 0;

	desc = aux + 1;
	aux = strchr(desc, '=');
	if (!aux) {
		printf("Invalid desc\n");
		return -1;
	}

	*aux = '\0';
	if (strcmp(desc, "options")) {
		printf("Unknown paramenter %s\n", desc);
		return -1;
	}

	e->e_options = desc;
	return 0;
}

static int add_element(struct mosaic *m, char *desc)
{
	struct element *e;
	struct tessera *t;
	char *aux;

	e = malloc(sizeof(*e));
	e->e_age = 0;
	e->e_at = NULL;
	e->e_options = NULL;

	aux = strchr(desc, ':');
	if (!aux) {
		printf("Missing parts in element\n");
		return -1;
	}

	*aux = '\0';
	t = find_tessera(ms, desc);
	if (!t) {
		printf("Unknown tessera %s\n", desc);
		return -1;
	}

	e->t = t;

	if (set_element(e, aux + 1))
		return -1;

	list_add_tail(&e->ml, &m->elements);
	return 0;
}

static int maybe_add_elements(struct mosaic *m, int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++)
		if (add_element(m, argv[i]))
			return -1;

	return 0;
}

static int update_element(struct mosaic *m, char *desc)
{
	char *aux;
	struct element *e;

	aux = strchr(desc, ':');
	if (!aux) {
		printf("Missing parts in element\n");
		return -1;
	}

	*aux = '\0';

	list_for_each_entry(e, &m->elements, ml) {
		if (strcmp(e->t->t_name, desc))
			continue;

		aux++;

		if (!strcmp(aux, "del")) {
			list_del(&e->ml);
			free(e);
			return 0;
		}

		return set_element(e, aux);
	}

	if (aux)
		*aux = ':';

	return add_element(m, desc);
}

static int update_elements(struct mosaic *m, int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++)
		if (update_element(m, argv[i]))
			return -1;

	return 0;
}

static int add_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic add <name> [<elements>]\n");
		return 1;
	}

	m = malloc(sizeof(*m));
	m->m_name = argv[0];
	INIT_LIST_HEAD(&m->elements);

	if (maybe_add_elements(m, argc - 1, argv + 1)) {
		free(m);
		return 1;
	}

	list_add_tail(&m->sl, &ms->mosaics);

	return config_update();
}

static int del_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic del <name>\n");
		return 1;
	}

	m = find_mosaic(argv[0]);
	if (!m) {
		printf("Unknown mosaic %s\n", argv[0]);
		return 1;
	}

	list_del(&m->sl);
	/* FIXME: del elements when going lib */
	free(m);

	return config_update();
}

static int change_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic change <name> [<elements>]\n");
		return 1;
	}

	m = find_mosaic(argv[0]);
	if (!m) {
		printf("Unknown mosaic %s\n", argv[0]);
		return 1;
	}

	if (update_elements(m, argc - 1, argv + 1))
		return 1;

	return config_update();
}

static int do_umount_mosaic(struct mosaic *m);

static int do_mount_mosaic_at(struct mosaic *m, char *mp_path, char *options)
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
	do_umount_mosaic(m);
	return -1;
}

static int do_umount_mosaic(struct mosaic *m)
{
	printf("NOT IMPLEMENTED\n");
	return 1;
}

static int mount_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 2) {
		printf("Usage: moctl mosaic mount <name> <location> [<options>]\n");
		return 1;
	}

	m = find_mosaic(argv[0]);
	if (!m) {
		printf("Unknown mosaic %s\n", argv[0]);
		return 1;
	}

	return do_mount_mosaic_at(m, argv[1], argv[2]) == 0 ? 0 : -1;
}

static int umount_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic umount <name>\n"); /* FIXME: location? */
		return 1;
	}

	m = find_mosaic(argv[0]);
	if (!m) {
		printf("Unknown mosaic %s\n", argv[0]);
		return 1;
	}

	return do_umount_mosaic(m) == 0 ? 0 : -1;
}

int do_mosaic(int argc, char **argv)
{
	if (argc < 1) {
		printf("Usage: moctl mosaic <list|show|add|del|change|mount|umount> ...\n");
		return 1;
	}

	if (argv_is(argv[0], "list"))
		return list_mosaics();
	if (argv_is(argv[0], "show"))
		return show_mosaic(argc - 1, argv + 1);
	if (argv_is(argv[0], "add"))
		return add_mosaic(argc - 1, argv + 1);
	if (argv_is(argv[0], "del"))
		return del_mosaic(argc - 1, argv + 1);
	if (argv_is(argv[0], "change"))
		return change_mosaic(argc - 1, argv + 1);
	if (argv_is(argv[0], "mount"))
		return mount_mosaic(argc - 1, argv + 1);
	if (argv_is(argv[0], "umount"))
		return umount_mosaic(argc - 1, argv + 1);

	printf("Unknown mosaic action %s\n", argv[0]);
	return 1;
}
