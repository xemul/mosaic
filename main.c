#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "config.h"
#include "util.h"

struct mosaic_state *ms;

static void usage(void)
{
	printf(
"Usage: moctl <object> <action> [<options>]\n"
"\n"
" * Objects:\n"
"    m|mosaic\n"
"    t|tessera\n"
"\n"
" * Actions:\n"
"   Common:\n"
"    l|list\n"
"    s|show    <name>\n"
"\n"
"   Mosaic:\n"
"    a|add     <name> [<elements>]\n"
"    d|del     <name>\n"
"    c|change  <name> [<elements>]\n"
"    m|mount   <name> <location> [<mount-options>]\n"
"\n"
"     Element: <name>:<age>:<location>[:options=<mount-options>]\n"
"              for \"change\" <name>:del to remove\n"
"\n"
"   Tessera:\n"
"    a|add     <name> <type> [<type-options>]\n"
"    d|del     <name>\n"
"    m|mount   <name>:<age> <location> [<mount-options>]\n"
"    g|grow    <name> <new-age>[:<base-age>]\n"
);
}

static int print_mosaic(struct mosaic *m, void *x)
{
	printf("%s\n", m->m_name);
	return 0;
}

static int list_mosaics(void)
{
	return mosaic_iterate(print_mosaic, NULL);
}

static int print_element(struct mosaic *m, struct element *e, void *x)
{
	printf("  - name: %s\n", e->t->t_name);
	if (e->e_age == 0)
		printf("    age:     base\n");
	else
		printf("    age:     %d\n", e->e_age);
	printf("    at:      %s\n", e->e_at ? : "-");
	printf("    options: %s\n", e->e_options ? : "none");
	printf("\n");

	return 0;
}

static int show_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic show <name>\n");
		return 1;
	}

	m = mosaic_find_by_name(argv[0]);
	if (!m) {
		printf("No such mosaic %s\n", argv[0]);
		return 1;
	}

	st_show_mounted(m);

	if (!list_empty(&m->elements))
		printf("elements:\n");

	return mosaic_iterate_elements(m, print_element, NULL);
}

static int set_elements(struct mosaic *m, int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++) {
		int age;
		char *at, *ed, *aux;

		/* Name */
		aux = strchr(argv[1], ':');
		if (!aux)
			goto err;

		*aux = '\0';

		/* Age */
		ed = aux + 1;

		if (!strcmp(ed, "del")) {
			if (mosaic_del_element(m, argv[1]))
				return 1;

			continue;
		}

		aux = strchr(ed, ':');
		if (!aux)
			goto err;

		age = atoi(ed);
		ed = aux + 1;

		/* Location */
		aux = strchr(ed, ':');
		if (aux)
			*aux = '\0';

		at = ed;

		/* Options */
		if (aux)
			ed = aux + 1;
		else
			ed = NULL;

		if (mosaic_set_element(m, argv[i], age, at, ed))
			return 1;
	}

	return 0;

err:
	printf("Element format error (name:age:location[:options]\n");
	return 1;
}

static int add_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic add <name> [<elements>]\n");
		return 1;
	}

	m = mosaic_new(argv[0]);
	if (!m)
		return 1;

	if (set_elements(m, argc - 1, argv + 1))
		return 1;

	return mosaic_add(m);
}

static int change_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic change <name> [<elements>]\n");
		return 1;
	}

	m = mosaic_find_by_name(argv[0]);
	if (!m) {
		printf("Unknown mosaic %s\n", argv[0]);
		return 1;
	}

	if (set_elements(m, argc - 1, argv + 1))
		return 1;

	return mosaic_update(m);
}

static int del_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic del <name>\n");
		return 1;
	}

	m = mosaic_find_by_name(argv[0]);
	if (!m) {
		printf("Unknown mosaic %s\n", argv[0]);
		return 1;
	}

	return mosaic_del(m);
}

static int mount_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 2) {
		printf("Usage: moctl mosaic mount <name> <location> [<options>]\n");
		return 1;
	}

	m = mosaic_find_by_name(argv[0]);
	if (!m) {
		printf("Unknown mosaic %s\n", argv[0]);
		return 1;
	}

	return mosaic_mount(m, argv[1], argv[2]) == 0 ? 0 : -1;
}

static int umount_mosaic(int argc, char **argv)
{
	struct mosaic *m;

	if (argc < 1) {
		printf("Usage: moctl mosaic umount <name> [<locaction>]\n");
		return 1;
	}

	m = mosaic_find_by_name(argv[0]);
	if (!m) {
		printf("Unknown mosaic %s\n", argv[0]);
		return 1;
	}

	return mosaic_umount(m, argv[1]) == 0 ? 0 : -1;
}

static int do_mosaic(int argc, char **argv)
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

int main(int argc, char **argv)
{
	ms = mosaic_parse_config("mosaic.conf");
	if (!ms) {
		fprintf(stderr, "Error loading config file\n");
		return 1;
	}

	if (argc < 2) {
		usage();
		return 1;
	}

	if (argv_is(argv[1], "mosaic"))
		return do_mosaic(argc - 2, argv + 2);
	if (argv_is(argv[1], "tessera"))
		return do_tessera(argc - 2, argv + 2);

	printf("Unknown action %s\n", argv[1]);
	return 1;
}
