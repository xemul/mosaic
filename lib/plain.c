#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "log.h"
#include "config.h"

struct plain_tessera {
	char *pl_location;
};

static int add_plain(struct tessera *t, int n_opts, char **opts)
{
	struct plain_tessera *pt;

	if (n_opts < 1) {
		log("Usage: moctl tessera add <name> plain <location>\n");
		return -1;
	}

	if (access(opts[0], F_OK) < 0) {
		if (mkdir(opts[0], 0600) < 0) {
			loge("Can't make dir for plain tessera");
			return -1;
		}
	}

	t->priv = pt = malloc(sizeof(*pt));
	pt->pl_location = strdup(opts[0]);

	return 0;
}

static void del_plain(struct tessera *t, int age)
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

static inline void print_plain_info(FILE *f, int off, struct plain_tessera *pt)
{
	fprintf(f, "%*slocation: %s\n", off, "", pt->pl_location);
}

static void save_plain(struct tessera *t, FILE *f)
{
	print_plain_info(f, CFG_TESS_OFF, t->priv);
}

static void show_plain(struct tessera *t, int age)
{
	if (age == -1)
		print_plain_info(stdout, 0, t->priv);
}

static int mount_plain(struct tessera *t, int age, char *path, char *options)
{
	struct plain_tessera *pt = t->priv;

	if (age != 0)
		return -1;

	if (options) {
		log("Mount options are not yet supported\n");
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
