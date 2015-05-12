#ifndef __MOSAIC_YAML_UTIL_H__
#define __MOSAIC_YAML_UTIL_H__
int yaml_parse_block(yaml_parser_t *p, int bs, int be, int (*body)(yaml_parser_t *, void *), void *x);
int yaml_parse_block_seq(yaml_parser_t *p, int bs, int be,
		int (*element)(yaml_parser_t *, void *), void *x);
int yaml_parse_map_body(yaml_parser_t *p, int (*parse_value)(yaml_parser_t *p, char *key, void *), void *x);
char *yaml_parse_scalar(yaml_parser_t *p);

int yaml_parse_all(yaml_parser_t *, int (*doc)(yaml_parser_t *, void *), void *x);
#endif
