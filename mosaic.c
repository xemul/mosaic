#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mosaic.h"
#include "tessera.h"
#include "util.h"

static void show_mosaic(struct mosaic *m)
{
	struct element *e;

	printf("name: %s\n", m->m_name);
	list_for_each_entry(e, &m->elements, ml) {
		printf("\ttessera: %s\n", e->t->t_name);
		if (e->e_age == AGE_LAST)
			printf("\tage:     latest\n");
		else
			printf("\tage:     %d\n", e->e_age);
		printf("\tat:      %s\n", e->e_at ? : "-");
		printf("\toptions: %s\n", e->e_options ? : "none");

		printf("\n");
	}
}

static int list_mosaics(void)
{
	struct mosaic *m;

	list_for_each_entry(m, &ms->mosaics, sl)
		show_mosaic(m);

	return 0;
}

static struct mosaic *find_mosaic(char *name)
{
	struct mosaic *m;

	list_for_each_entry(m, &ms->mosaics, sl)
		if (!strcmp(m->m_name, name))
			return m;

	return NULL;
}

static int set_element(struct element *e, char *aux)
{
	char *desc;

	while (aux) {
		char *aux2;

		desc = aux + 1;
		aux = strchr(desc, '=');
		if (!aux) {
			printf("Element format error %s\n", desc);
			return -1;
		}

		*aux = '\0';
		aux++;
		aux2 = strchr(aux, ':');
		if (aux2)
			*aux2 = '\0';

		if (!strcmp(desc, "age"))
			e->e_age = atoi(aux);
		else if (!strcmp(desc, "at"))
			e->e_at = aux;
		else if (!strcmp(desc, "options"))
			e->e_options = aux;
		else {
			printf("Element format error %s\n", desc);
			return -1;
		}

		aux = aux2;
	}

	return 0;
}

static int add_element(struct mosaic *m, char *desc)
{
	struct element *e;
	struct tessera *t;
	char *aux;

	/*
	 * Element description is
	 *
	 *  name:age=$age:at=$path:options=$opts
	 *
	 * All but name is optional
	 */

	e = malloc(sizeof(*e));
	e->e_age = AGE_LAST;
	e->e_at = NULL;
	e->e_options = NULL;

	aux = strchr(desc, ':');
	if (aux)
		*aux = '\0';

	t = find_tessera(ms, desc);
	if (!t) {
		printf("Unknown tessera %s\n", desc);
		return -1;
	}

	e->t = t;

	if (set_element(e, aux))
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
	if (aux)
		*aux = '\0';

	list_for_each_entry(e, &m->elements, ml) {
		if (strcmp(e->t->t_name, desc))
			continue;

		if (!aux) {
			printf("Can't update element to none\n");
			return 1;
		}

		if (!strcmp(aux + 1, "del")) {
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
		printf("Usage: moctl mosaic add [name] <elements>\n");
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
		printf("Usage: moctl mosaic del [name]\n");
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
		printf("Usage: moctl mosaic change [name] <elements>\n");
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

int do_mosaic(int argc, char **argv)
{
	if (argc < 1) {
		printf("Usage: moctl mosaic [list|add|del|change] ...\n");
		return 1;
	}

	if (argv_is(argv[0], "list"))
		return list_mosaics();
	if (argv_is(argv[0], "add"))
		return add_mosaic(argc - 1, argv + 1);
	if (argv_is(argv[0], "del"))
		return del_mosaic(argc - 1, argv + 1);
	if (argv_is(argv[0], "change"))
		return change_mosaic(argc - 1, argv + 1);

	printf("Unknown mosaic action %s\n", argv[0]);
	return 1;
}
