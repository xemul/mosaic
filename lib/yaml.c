#include <yaml.h>
#include "yaml-util.h"

/******************************************************
 * Helpers to event-based libyaml parsing
 ******************************************************/

/*
 * Helper to parse the file between bs and be events.
 */
int yaml_parse_block(yaml_parser_t *p, int bs, int be, int (*body)(yaml_parser_t *, void *), void *x)
{
	yaml_event_t e;
	int ret = -1;

	if (!yaml_parser_parse(p, &e))
		goto err;

	if (e.type != bs) {
		/*
		 * This can be called to get "next" elemt
		 * which may happen to be the "wrong" one.
		 *
		 * Typically this will get propagated to
		 * the previous yaml_parse_block() and
		 * should happen to be that's be value.
		 */
		ret = e.type;
		goto err_e;
	}

	yaml_event_delete(&e);

	ret = body(p, x);
	if (ret != be)
		goto err;

	/*
	 * All the YAML_FOO constants are positive.
	 * Thus zero means OK, negative means error.
	 */
	return 0;

err_e:
	yaml_event_delete(&e);
err:
	return ret;
}

/*
 * Helper to parse list of bs:be blocks
 */
int yaml_parse_block_seq(yaml_parser_t *p, char *typ, int bs, int be,
		int (*element)(yaml_parser_t *, void *), void *x)
{
	int ret;

	do {
		ret = yaml_parse_block(p, bs, be, element, x);
	} while (ret == 0);

	return ret;
}

/*
 * Helper to parse body of the mapping. Body
 * is the list of scalar name vs value pairs.
 *
 * Value is parsed in a callback.
 */
int yaml_parse_map_body(yaml_parser_t *p, int (*parse_value)(yaml_parser_t *p, char *key, void *), void *x)
{
	int ret = -1;
	yaml_event_t e;

	while (1) {
		if (!yaml_parser_parse(p, &e))
			break;

		if (e.type != YAML_SCALAR_EVENT) {
			ret = e.type;
			yaml_event_delete(&e);
			break;
		}

		ret = parse_value(p, (char *)e.data.scalar.value, x);
		yaml_event_delete(&e);

		if (ret)
			break;
	}

	return ret;
}

/*
 * Helper to parse scalar value
 */
char *yaml_parse_scalar(yaml_parser_t *p)
{
	yaml_event_t e;
	char *ret = NULL;

	if (!yaml_parser_parse(p, &e))
		return NULL;
	if (e.type == YAML_SCALAR_EVENT)
		ret = strdup((char *)e.data.scalar.value);
	yaml_event_delete(&e);
	return ret;
}

struct ypd {
	int (*doc)(yaml_parser_t *, void *);
	void *x;
};

static int parse_document(yaml_parser_t *p, void *_x)
{
	struct ypd *x = _x;

	return x->doc(p, x->x);
}

static int parse_stream(yaml_parser_t *p, void *x)
{
	return yaml_parse_block(p, YAML_DOCUMENT_START_EVENT, YAML_DOCUMENT_END_EVENT,
			parse_document, x);
}

int yaml_parse_all(yaml_parser_t *p, int (*doc)(yaml_parser_t *, void *), void *x)
{
	struct ypd _x = {
		.doc = doc,
		.x = x,
	};

	return yaml_parse_block(p, YAML_STREAM_START_EVENT, YAML_STREAM_END_EVENT,
			parse_stream, &_x);
}
