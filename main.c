#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "config.h"

static struct mosaic_state *ms;

static inline int argv_is(char *argv, char *is)
{
	while (1) {
		if (*argv == '\0')
			return 1;
		if (*is == '\0')
			return 0;
		if (*is != *argv)
			return 0;

		is++;
		argv++;
	}
}

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
		printf("\tat:      %s\n", e->e_at);
		printf("\toptions: %s\n", e->e_options ? : "None");

		printf("\n");
	}
}

static int list_mosaics(int argc, char **argv)
{
	struct mosaic *m;

	list_for_each_entry(m, &ms->mosaics, sl)
		show_mosaic(m);

	return 0;
}

static char *t_types[] = {
	[ TESSERA_OVERLAY ] = "overlay",
	[ TESSERA_BTRFS ] = "btrfs",
	[ TESSERA_DMTHIN ] = "dmthin",
};

static void show_tessera(struct tessera *t)
{
	printf("name: %s\n", t->t_name);
	printf("type: %s\n", t_types[t->t_type]);
	printf("\n");
}

static int list_tesserae(int argc, char **argv)
{
	struct tessera *t;

	list_for_each_entry(t, &ms->tesserae, sl)
		show_tessera(t);

	return 0;
}

static int mosaic_list(int argc, char **argv)
{
	if (argc == 0)
		goto out;

	if (argv_is(argv[0], "mosaic"))
		return list_mosaics(argc - 1, argv + 1);
	if (argv_is(argv[0], "tessera"))
		return list_tesserae(argc - 1, argv + 1);

	printf("Unknown object %s\n", argv[0]);
out:
	return 1;
}

int main(int argc, char **argv)
{
	ms = mosaic_parse_config("mosaic.conf");
	if (!ms) {
		fprintf(stderr, "Error loading config file\n");
		return 1;
	}

	if (argc < 2)
		return 1;

	if (argv_is(argv[1], "list"))
		return mosaic_list(argc - 2, argv + 2);

	printf("Unknown action %s\n", argv[1]);
	return 1;
}
