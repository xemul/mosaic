#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mosaic.h"
#include "tessera.h"

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

int list_mosaics(void)
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

int add_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: mosaic add mosaic [name] <elements>\n");
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

int del_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: mosaic del mosaic [name]\n");
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
