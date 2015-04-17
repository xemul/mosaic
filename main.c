#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "config.h"

static void show_state(struct mosaic_state *ms)
{
	struct mosaic *m;
	struct tessera *t;

	printf("Mosaics:\n");
	list_for_each_entry(m, &ms->mosaics, sl) {
		struct element *e;

		printf("\t%s:\n", m->m_name);
		list_for_each_entry(e, &m->elements, ml)
			printf("\t\t%s.%d@%s(%s)\n",
					e->t->t_name, e->e_age, e->e_at, e->e_options);
	}

	printf("Tesserae:\n");
	list_for_each_entry(t, &ms->tesserae, sl)
		printf("\t%d:%s\n", t->t_type, t->t_name);
}

int main(int argc, char **argv)
{
	struct mosaic_state *ms;

	ms = mosaic_parse_config(argv[1]);

	show_state(ms);

	return 0;
}
