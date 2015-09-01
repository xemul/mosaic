#ifndef __MOSAIC_UAPI_H__
#define __MOSAIC_UAPI_H__

/*
 * Mosaic management
 */

typedef struct mosaic *mosaic_t;
mosaic_t mosaic_open(const char *cfg, int open_flags);
void mosaic_close(mosaic_t m);

int mosaic_mount(mosaic_t m, char *path, int mount_flags);
void mosaic_set_tessera_fs(mosaic_t m, char *fsname);

/*
 * Tessera management
 */

typedef struct tessera *tessera_t;
tessera_t mosaic_open_tess(mosaic_t m, char *name, int open_flags);
void mosaic_close_tess(tessera_t);

int mosaic_make_tess(mosaic_t m, char *name, unsigned long size_in_blocks, int make_flags);
int mosaic_make_tess_fs(mosaic_t m, char *name, unsigned long size_in_blocks, char *fsname, int make_flags);

int mosaic_clone_tess(tessera_t from, char *name, int clone_flags);
int mosaic_drop_tess(tessera_t t, int drop_flags);
int mosaic_mount_tess(tessera_t t, char *path, int mount_flags);
int mosaic_resize_tess(tessera_t t, unsigned long new_size_in_blocks, int resize_flags);

/* Misc */
typedef void (*mosaic_log_fn)(const char *f, ...)
	            __attribute__ ((format(printf, 1, 2)));
#endif
