#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "config.h"
#include "util.h"

extern struct tess_desc tess_desc_overlay;

static struct tess_desc tess_desc_btrfs = {
	.td_name = "btrfs",
};

static struct tess_desc tess_desc_dmthin = {
	.td_name = "dmthin",
};

struct tess_desc *tess_desc_by_type(char *type)
{
	if (!strcmp(type, "overlay"))
		return &tess_desc_overlay;
	if (!strcmp(type, "btrfs"))
		return &tess_desc_btrfs;
	if (!strcmp(type, "dm_thin"))
		return &tess_desc_dmthin;

	return NULL;
}

struct tessera *find_tessera(struct mosaic_state *ms, char *name)
{
	struct tessera *t;

	list_for_each_entry(t, &ms->tesserae, sl)
		if (!strcmp(t->t_name, name))
			return t;

	return NULL;
}

static int show_tessera(int argc, char **argv)
{
	struct tessera *t;

	if (argc < 1) {
		printf("Usage: moctl tessera show [name]\n");
		return 1;
	}

	t = find_tessera(ms, argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	printf("type: %s\n", t->t_desc->td_name);
	if (t->t_desc->show)
		t->t_desc->show(t);

	return 0;
}

static int list_tesserae(void)
{
	struct tessera *t;

	list_for_each_entry(t, &ms->tesserae, sl)
		printf("%s\n", t->t_name);

	return 0;
}

static int add_tessera(int argc, char **argv)
{
	int i;
	struct tess_desc *td;

	if (argc < 2) {
		printf("Usage: moctl tessera add [type] [name] ...\n");
		goto out;
	}

	td = tess_desc_by_type(argv[0]);
	if (!td || !td->add)
		goto out_t;

	if (td->add(td, argv[1], argc - 2, argv + 2))
		return 1;

	return config_update();

out_t:
	printf("Unknown tessera type %s\n", argv[0]);
out:
	return 1;
}

static int del_tessera(int argc, char **argv)
{
	struct tessera *t;

	if (argc < 1) {
		printf("Usage: moctl tessera del [name]\n");
		return 1;
	}

	t = find_tessera(ms, argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	list_del(&t->sl);
	if (t->t_desc->del)
		t->t_desc->del(t->t_desc, t);
	free(t);

	return config_update();
}

int do_mount_tessera_at(struct tessera *t, int age, char *path, char *options)
{
	if (!t->t_desc->mount) {
		printf("Mounting of %s is not supported\n",
				t->t_desc->td_name);
		return -1;
	}

	return t->t_desc->mount(t, age, path, options);
}

static int mount_tessera(int argc, char **argv)
{
	struct tessera *t;
	struct stat buf;
	int age = 0;
	char *options = NULL, *aux;

	if (argc < 2) {
		printf("Usage: moctl tessera mount [name]:[age] [location] <options>\n");
		return 1;
	}

	aux = strchr(argv[0], ':');
	if (aux) {
		*aux = '\0';
		age = atoi(aux + 1);
	}

	if (argc >= 3)
		options = argv[2];

	if (stat(argv[1], &buf)) {
		printf("Can't stat %s\n", argv[1]);
		return 1;
	}

	if (!S_ISDIR(buf.st_mode)) {
		printf("Can't mount mosaic on non-directory\n");
		return 1;
	}

	t = find_tessera(ms, argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	return do_mount_tessera_at(t, age, argv[1], options);
}

static int do_grow_tessera(struct tessera *t, int base_age, int new_age)
{
	if (!t->t_desc->grow) {
		printf("Growing is not supported for %s\n", t->t_desc->td_name);
		return 1;
	}

	return t->t_desc->grow(t, base_age, new_age);
}

static int grow_tessera(int argc, char **argv)
{
	struct tessera *t;
	int base_age = 0, new_age = 1;

	if (argc < 2) {
		printf("Usage: moctl tessera grow [name] <base-age:[new-age]>\n");
		return 1;
	}

	t = find_tessera(ms, argv[0]);
	if (!t) {
		printf("Unknown tessera %s\n", argv[0]);
		return 1;
	}

	if (argc >= 2) {
		char *aux;

		aux = strchr(argv[1], ':');
		if (aux) {
			*aux = '\0';
			new_age = atoi(aux + 1);
		}

		base_age = atoi(argv[1]);
	}

	return do_grow_tessera(t, base_age, new_age);
}

int do_tessera(int argc, char **argv)
{
	if (argc < 1) {
		printf("Usage: moctl tessera [list|show|add|del|mount|grow] ...\n");
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
	if (argv_is(argv[0], "grow"))
		return grow_tessera(argc - 1, argv + 1);

	printf("Unknown mosaic action %s\n", argv[0]);
	return 1;
}
