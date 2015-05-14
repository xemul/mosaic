#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"

struct plain_tessera {
	char *pl_location;
};

static int add_plain(struct tessera *t, int n_opts, char **opts)
{
	struct plain_tessera *pt;

	if (n_opts < 1) {
		printf("Usage: moctl tessera add <name> plain <location>\n");
		return -1;
	}

	if (access(opts[0], F_OK) < 0) {
		if (mkdir(opts[0], 0600) < 0) {
			perror("Can't make dir for plain tessera");
			return -1;
		}
	}

	t->priv = pt = malloc(sizeof(*pt));
	pt->pl_location = strdup(opts[0]);

	return 0;
}

static void del_plain(struct tessera *t)
{
	struct plain_tessera *pt = t->priv;

	free(pt->pl_location);
	free(pt);
}

static int parse_plain(struct tessera *t, char *key, char *val)
{
	struct plain_tessera *pt = t->priv;

	if (!pt) {
		pt = malloc(sizeof(*pt));
		pt->pl_location = NULL;
		t->priv = pt;
	}

	if (!key)
		return pt->pl_location ? 0 : -1;

	if (!strcmp(key, "location")) {
		pt->pl_location = val;
		return 0;
	}

	return -1;
}

static void save_plain(struct tessera *t, FILE *f)
{
	struct plain_tessera *pt = t->priv;

	fprintf(f, "    location: %s\n", pt->pl_location);
}

static void show_plain(struct tessera *t)
{
	struct plain_tessera *pt = t->priv;
	printf("location: %s\n", pt->pl_location);
}

static int mount_plain(struct tessera *t, int age, char *path, char *options)
{
	struct plain_tessera *pt = t->priv;

	if (age != 0)
		return -1;

	if (options) {
		printf("Mount options are not yet supported\n");
		return -1;
	}

	return mount(pt->pl_location, path, NULL, MS_BIND, NULL);
}

struct tess_desc tess_desc_plain = {
	.td_name = "plain",
	.add = add_plain,
	.del = del_plain,
	.parse = parse_plain,
	.save = save_plain,
	.show = show_plain,
	.mount = mount_plain,
	/*
	 * Growing can only be implemented by
	 * copying the whole subtree. Not sure
	 * anyone needs this.
	 */
};
