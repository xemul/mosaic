#ifndef __MOSAIC_UAPI_H__
#define __MOSAIC_UAPI_H__

/*
 * Mosaic management
 */

typedef struct mosaic *mosaic_t;
mosaic_t mosaic_open(const char *cfg, int open_flags);
void mosaic_close(mosaic_t m);

int mosaic_mount(mosaic_t m, char *path, int mount_flags);

/*
 * Tessera management
 */

typedef struct tessera *tessera_t;
tessera_t mosaic_open_tess(mosaic_t m, char *name, int open_flags);
void mosaic_close_tess(tessera_t);

/*
 * Make new tesserae. The first one makes raw block device, the
 * second one also puts filesystem on it.
 */
int mosaic_make_tess(mosaic_t m, char *name, unsigned long size_in_blocks, int make_flags);
int mosaic_make_tess_fs(mosaic_t m, char *name, unsigned long size_in_blocks, int make_flags);

/*
 * Create a clone of existing tesserae with COW (when possible)
 */
int mosaic_clone_tess(tessera_t from, char *name, int clone_flags);

int mosaic_drop_tess(tessera_t t, int drop_flags);
int mosaic_resize_tess(tessera_t t, unsigned long new_size_in_blocks, int resize_flags);

/*
 * Mounting and umounting of tessera
 */
int mosaic_mount_tess(tessera_t t, char *path, int mount_flags);
int mosaic_umount_tess(tessera_t t, char *path, int umount_flags);

/*
 * Getting path to tessera block device (when possible)
 * and putting it back.
 *
 * Return value from the first one is the lenght of the name
 * of the device (even if it doesn't fit the buffer len).
 */
int mosaic_get_tess_bdev(tessera_t t, char *devs, int len, int flags);
int mosaic_put_tess_bdev(tessera_t t, char *devs);

int mosaic_get_tess_size(tessera_t t, unsigned long *size_in_blocks);

/* Misc */
typedef void (*mosaic_log_fn)(const char *f, ...)
	            __attribute__ ((format(printf, 1, 2)));
#endif
