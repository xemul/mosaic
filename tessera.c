#include <stdio.h>
#include <stdlib.h>
#include "mosaic.h"
#include "tessera.h"
#include "config.h"
#include "util.h"

static int add_overlay(struct tess_desc *me, char *name, int argc, char **argv)
{
	struct tessera *t;

	if (argc != 0) {
		printf("Unknown args for overlay %s...\n", argv[0]);
		return 1;
	}

	t = malloc(sizeof(*t));
	t->t_name = name;
	t->t_desc = me;
	list_add_tail(&t->sl, &ms->tesserae);

	return 0;
}

static int mount_overlay(struct tessera *t, int age, char *path, char *options)
{
	printf("NOT IMPLEMENTED\n");
	return 1;
}

static struct tess_desc tess_desc_overlay = {
	.td_name = "overlay",
	.add = add_overlay,
	.mount = mount_overlay,
};

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

int do_tessera(int argc, char **argv)
{
	if (argc < 1) {
		printf("Usage: moctl tessera [list|show|add|del] ...\n");
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

	printf("Unknown mosaic action %s\n", argv[0]);
	return 1;
}
