#include <stdio.h>
#include "mosaic.h"

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

int list_mosaics(void)
{
	struct mosaic *m;

	list_for_each_entry(m, &ms->mosaics, sl)
		show_mosaic(m);

	return 0;
}

int add_mosaic(int argc, char **argv)
{
	return 1;
}
