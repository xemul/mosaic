#include <stdio.h>
#include "yaml-util.h"
#include "mosaic.h"

static int parse_config(yaml_parser_t *p, void *x)
{
	return 0;
}

int mosaic_parse_config(const char *cfg, struct mosaic *m)
{
	FILE *cfg_f;
	yaml_parser_t parser;
	int ret = -1;

	cfg_f = fopen(cfg, "r");
	if (!cfg_f)
		goto out;

	if (!yaml_parser_initialize(&parser))
		goto out_c;

	yaml_parser_set_input_file(&parser, cfg_f);
	ret = yaml_parse_all(&parser, parse_config, m);
	yaml_parser_delete(&parser);
out_c:
	fclose(cfg_f);
out:
	return ret;
}
