#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "log.h"
#include "thin-internal.h"

#ifdef DOTEST
#define DB_DIRNAME	"."
#endif

#ifndef DB_DIRNAME
#define DB_DIRNAME	"/var/run/mosaic/"
#endif

static inline void dbp(char *name, char *p, int ps)
{
	snprintf(p, ps, DB_DIRNAME "/%s.db", name);
}

static inline void dbup(char *name, char *p, int ps)
{
	snprintf(p, ps, DB_DIRNAME "/.%s.upd", name);
}

static FILE *open_db(char *name, FILE **update_db)
{
	char path[PATH_MAX];
	FILE *db;

	/* FIXME: locking */

	dbp(name, path, sizeof(path));
	db = fopen(path, "r");
	if (!db) {
		loge("Can't open database: %m\n");
		return NULL;
	}

	if (update_db) {
		FILE *udb;

		dbup(name, path, sizeof(path));
		udb = fopen(path, "w");
		if (!udb) {
			loge("Can't open update base: %m\n");
			fclose(db);
			return NULL;
		}

		*update_db = udb;
	}

	return db;
}

static int close_db(char *name, FILE *db, FILE *udb, bool updated)
{
	fclose(db);

	if (udb) {
		char upath[PATH_MAX];

		fclose(udb);
		dbup(name, upath, sizeof(upath));

		if (updated) {
			char path[PATH_MAX];

			dbp(name, path, sizeof(path));
			if (rename(upath, path)) {
				loge("Can't update database: %m\n");
				return -1;
			}
		} else {
			if (unlink(upath))
				logw("Can't remove update database: %m\n");
		}
	}

	return 0;
}

int thin_id_new(char *m_name, char *v_name, unsigned *ret_id)
{
	FILE *db, *udb;
	char str[1024];
	unsigned max_id = 0;

	db = open_db(m_name, &udb);
	if (!db)
		return -1;

	while (fgets(str, sizeof(str), db)) {
		char *aux, *e;
		unsigned id;

		aux = strchr(str, ':');
		if (!aux)
			goto fatal;

		id = strtol(aux + 1, &e, 10);
		if (*e != '\n')
			goto fatal;

		*aux = '\0';
		if (strcmp(v_name, str) != 0) {
			/* FIXME -- IDs can overflow */
			if (id > max_id)
				max_id = id;
			fprintf(udb, "%s:%u\n", str, id);
			continue;
		}

		*ret_id = id;
		close_db(m_name, db, udb, false);
		return 1;
	}

	max_id++;
	fprintf(udb, "%s:%u\n", v_name, max_id);
	*ret_id = max_id;
	return close_db(m_name, db, udb, true) ? : 0;

fatal:
	loge("RT database corrupted at [%s]\n", str);
	close_db(m_name, db, udb, false);
	return -1;
}

int thin_id_get(char *m_name, char *v_name, unsigned *ret_id)
{
	FILE *db;
	char str[1024];
	int found = 0;

	db = open_db(m_name, NULL);
	if (!db)
		return -1;

	while (fgets(str, sizeof(str), db)) {
		char *aux, *e;
		unsigned id;

		aux = strchr(str, ':');
		if (!aux)
			goto fatal;

		id = strtol(aux + 1, &e, 10);
		if (*e != '\n')
			goto fatal;

		*aux = '\0';
		if (strcmp(v_name, str) != 0)
			continue;

		*ret_id = id;
		found = 1;
		break;
	}

	close_db(m_name, db, NULL, false);
	return found;

fatal:
	loge("RT database corrupted at [%s]\n", str);
	close_db(m_name, db, NULL, false);
	return -1;
}

int thin_id_del(char *m_name, char *v_name)
{
	FILE *db, *udb;
	char str[1024];
	int found = 0;

	db = open_db(m_name, &udb);
	if (!db)
		return -1;

	while (fgets(str, sizeof(str), db)) {
		char *aux, *e;
		unsigned id;

		aux = strchr(str, ':');
		if (!aux)
			goto fatal;

		id = strtol(aux + 1, &e, 10);
		if (*e != '\n')
			goto fatal;

		*aux = '\0';
		if (found || strcmp(v_name, str) != 0) {
			fprintf(udb, "%s:%u\n", str, id);
			continue;
		}

		/* Found one, skip from update */
		found = 1;
	}

	return close_db(m_name, db, udb, true) ? : found;

fatal:
	loge("RT database corrupted at [%s]\n", str);
	close_db(m_name, db, udb, false);
	return -1;
}

#ifdef DOTEST
int main(int argc, char **argv)
{
	unsigned id = 0;
	int ret;

	if (!strcmp(argv[1], "new")) {
		ret = thin_id_new("test", argv[2], &id);
		if (ret < 0)
			printf("Failed\n");
		else
			printf("%s: %u\n", ret ? "old" : "new", id);
	} else if (!strcmp(argv[1], "get")) {
		ret = thin_id_get("test", argv[2], &id);
		if (ret < 0)
			printf("Failed\n");
		else
			printf("%s: %u\n", ret ? "found" : "NO", id);
	} else if (!strcmp(argv[1], "del")) {
		ret = thin_id_del("test", argv[2]);
		if (ret < 0)
			printf("Failed\n");
		else
			printf("%s\n", ret ? "found" : "NO");
	} else
		return -1;


	return 0;
}
#endif
