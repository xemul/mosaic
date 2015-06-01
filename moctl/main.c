#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uapi/mosaic.h"

/*
 * FIXME -- below is not the part of UAPI
 */
#include "mosaic.h"
#include "tessera.h"

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
"    u|umount  <name> [<location>]\n"
"\n"
"     Element: <name>:<age>:<location>[:options=<mount-options>]\n"
"              for \"change\" <name>:del to remove\n"
"\n"
"   Tessera:\n"
"    a|add     <name> <type> [<type-options>]\n"
"    d|del     <name>\n"
"    m|mount   <name>:<age> <location> [<mount-options>]\n"
"    u|umount  <name>:<age> [<location>]\n"
"    g|grow    <name> <new-age>[:<base-age>]\n"
"    t|types   -- shows available types\n"
);
}

/*
 * Mosaic CLI
 */

static int print_mosaic(struct mosaic *m, void *x)
{
	printf("%s\n", m->m_name);
	return 0;
}

static int list_mosaics(void)
{
	return mosaic_iterate(print_mosaic, NULL);
}

static int print_mounted(struct mosaic *m, char *path, void *x)
{
	int *pr = x;

	if (!*pr) {
		*pr = 1;
		printf("mounted:\n");
	}

	printf("  - %s\n", path);
	return 0;
}

static int print_element(struct mosaic *m, struct element *e, void *x)
{
	int *pr = x;

	if (!*pr) {
		*pr = 1;
		printf("elements:\n");
	}

	printf("  - name: %s\n", e->t->t_name);
	if (!e->e_age)
		printf("    age:     base\n");
	else
		printf("    age:     %s\n", e->e_age);
	printf("    at:      %s\n", e->e_at ? : "-");
	printf("    options: %s\n", e->e_options ? : "none");
	printf("\n");

	return 0;
}

static int show_mosaic(int argc, char **argv)
{
	struct mosaic *m;
	int printed;

	if (argc < 1) {
		printf("Usage: moctl mosaic show <name>\n");
		return 1;
	}

	m = mosaic_find_by_name(argv[0]);
	if (!m) {
		printf("No such mosaic %s\n", argv[0]);
		return 1;
	}

	printed = 0;
	mosaic_iterate_mounted(m, print_mounted, &printed);

	printed = 0;
	return mosaic_iterate_elements(m, print_element, &printed);
}

static int set_elements(struct mosaic *m, int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++) {
		char *age, *at, *ed, *aux;

		/* Name */
		aux = strchr(argv[i], ':');
		if (!aux)
			goto err;

		*aux = '\0';
		ed = aux + 1;

		/* Age */
		if (!strcmp(ed, "del")) {
			if (mosaic_del_element(m, argv[i]))
				return 1;

			continue;
		}

		aux = strchr(ed, ':');
		if (!aux)
			goto err;

		age = ed;
		*aux = '\0';
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

		if (mosaic_set_element(m, argv[i], age, at, ed)) {
			printf("Error setting element\n");
			return 1;
		}
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

	return mosaic_add(m) == 0 ? 0 : 1;
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

	return mosaic_update(m) == 0 ? 0 : 1;
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

	return mosaic_del(m) == 0 ? 0 : 1;
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

/*
 * Tessera CLI
 */

static int print_tessera(struct tessera *t, void *x)
{
	printf("%s\n", t->t_name);
	return 0;
}

static int list_tesserae(void)
{
	return mosaic_iterate_tesserae(print_tessera, NULL);
}

static int print_mounted_t(struct tessera *t, char *age, char *mp, void *x)
{
	int *p = x;

	if (*p == 1) {
		*p = 2;
		printf("    mounted:\n");
	}

	printf("      - %s\n", mp);
	return 0;
}

static int print_age(struct tessera *t, char *age, void *x)
{
	int *p = x;

	if (!*p) {
		*p = 1;
		printf("ages:\n");
	}

	printf("  - age: %s\n", age);
	mosaic_iterate_mounted_t(t, age, print_mounted_t, p);

	if (t->t_desc->show)
		t->t_desc->show(t, age);

	return 0;
}

static int show_tessera(int argc, char **argv)
{
	struct tessera *t;
	int printed;

	if (argc < 1) {
		printf("Usage: moctl tessera show <name>\n");
		return 1;
	}

	t = mosaic_find_tessera(argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	printed = 0;
	mosaic_iterate_ages_t(t, print_age, &printed);

	printf("type: %s\n", t->t_desc->td_name);
	if (t->t_desc->show)
		t->t_desc->show(t, NULL);

	return 0;
}

static int add_tessera(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: moctl tessera add <name> <type> ...\n");
		return 1;
	}

	return mosaic_add_tessera(argv[1], argv[0], argc - 2, argv + 2) == 0 ? 0 : 1;
}

static int del_tessera(int argc, char **argv)
{
	struct tessera *t;
	char *age = NULL /* "all" by default */, *aux;

	if (argc < 1) {
		printf("Usage: moctl tessera del <name>[:<age>]\n");
		return 1;
	}

	aux = strchr(argv[0], ':');
	if (aux) {
		*aux = '\0';
		age = aux + 1;
	}

	t = mosaic_find_tessera(argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	return mosaic_del_tessera(t, age) == 0 ? 0 : 1;
}

static int mount_tessera(int argc, char **argv)
{
	struct tessera *t;
	char *age = NULL, *options = NULL, *aux;

	if (argc < 2) {
		printf("Usage: moctl tessera mount <name>:<age> <location> [<options>]\n");
		return 1;
	}

	aux = strchr(argv[0], ':');
	if (aux) {
		*aux = '\0';
		age = aux + 1;
	}

	if (argc >= 3)
		options = argv[2];

	t = mosaic_find_tessera(argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	return mosaic_mount_tessera(t, age, argv[1], options) == 0 ? 0 : 1;
}

static int umount_tessera(int argc, char **argv)
{
	struct tessera *t;
	char *age = NULL, *aux;

	if (argc < 1) {
		printf("Usage: moctl tessera umount <name>:<age> [<location>]\n");
		return 1;
	}

	aux = strchr(argv[0], ':');
	if (aux) {
		*aux = '\0';
		age = aux + 1;
	}

	t = mosaic_find_tessera(argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	return mosaic_umount_tessera(t, age, argv[1]) == 0 ? 0 : 1;
}

static int grow_tessera(int argc, char **argv)
{
	struct tessera *t;
	char *base_age = NULL, *new_age, *aux;

	if (argc < 2) {
		printf("Usage: moctl tessera grow <name> <new-age>[:<base-age>]\n");
		return 1;
	}

	t = mosaic_find_tessera(argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	aux = strchr(argv[1], ':');
	if (aux) {
		*aux = '\0';
		base_age = aux + 1;
	}

	new_age = argv[1];

	return mosaic_grow_tessera(t, new_age, base_age) == 0 ? 0 : 1;
}

static int show_t_types(void)
{
	mosaic_print_types_t();
	return 0;
}

static int do_tessera(int argc, char **argv)
{
	if (argc < 1) {
		printf("Usage: moctl tessera <list|show|add|del|mount|grow|types> ...\n");
		return 1;
	}

	if (argv_is(argv[0], "list"))
		return list_tesserae();
	if (argv_is(argv[0], "show"))
		return show_tessera(argc - 1, argv + 1);
	if (argv_is(argv[0], "add"))
		return add_tessera(argc - 1, argv + 1);
	if (argv_is(argv[0], "del"))
		return del_tessera(argc - 1, argv + 1);
	if (argv_is(argv[0], "mount"))
		return mount_tessera(argc - 1, argv + 1);
	if (argv_is(argv[0], "umount"))
		return umount_tessera(argc - 1, argv + 1);
	if (argv_is(argv[0], "grow"))
		return grow_tessera(argc - 1, argv + 1);
	if (argv_is(argv[0], "types"))
		return show_t_types();

	printf("Unknown mosaic action %s\n", argv[0]);
	return 1;
}

static void print_lib_log(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
		return 1;
	}

	if (mosaic_init()) {
		printf("Error loading config file\n");
		return 1;
	}

	mosaic_init_err_log(print_lib_log);

	if (argv_is(argv[1], "mosaic"))
		return do_mosaic(argc - 2, argv + 2);
	if (argv_is(argv[1], "tessera"))
		return do_tessera(argc - 2, argv + 2);

	printf("Unknown action %s\n", argv[1]);
	return 1;
}
