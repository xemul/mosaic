#ifndef __MOSAIC_UAPI_H__
#define __MOSAIC_UAPI_H__

#pragma GCC visibility push(default)

/*
 * Mosaic management
 */

typedef struct mosaic *mosaic_t;
mosaic_t mosaic_open(const char *cfg, int open_flags);
void mosaic_close(mosaic_t m);

int mosaic_mount(mosaic_t m, const char *path, int mount_flags);

/*
 * Volume management
 */

typedef struct volume *volume_t;
volume_t mosaic_open_vol(mosaic_t m, const char *name, int open_flags);
void mosaic_close_vol(volume_t);

/*
 * Make new volumee. The first one makes raw block device, the
 * second one also puts filesystem on it.
 */
int mosaic_make_vol(mosaic_t m, const char *name, unsigned long size_in_blocks, int make_flags);
int mosaic_make_vol_fs(mosaic_t m, const char *name, unsigned long size_in_blocks, int make_flags);

/*
 * Create a clone of existing volumee with COW (when possible)
 */
int mosaic_clone_vol(volume_t from, const char *name, int clone_flags);

int mosaic_drop_vol(volume_t t, int drop_flags);
int mosaic_resize_vol(volume_t t, unsigned long new_size_in_blocks, int resize_flags);

/*
 * Mounting and umounting of volume
 */
int mosaic_mount_vol(volume_t t, const char *path, int mount_flags);
int mosaic_umount_vol(volume_t t, const char *path, int umount_flags);

/*
 * Getting path to volume's block device (when possible)
 * and putting it back.
 *
 * Return value from the first one is the lenght of the name
 * of the device (even if it doesn't fit the buffer len).
 */
int mosaic_get_vol_bdev(volume_t t, char *dev, int len, int flags);
int mosaic_put_vol_bdev(volume_t t);

int mosaic_get_vol_size(volume_t t, unsigned long *size_in_blocks);

/* Misc */
typedef void (*mosaic_log_fn)(const char *f, ...)
	__attribute__ ((format(printf, 1, 2)));

#pragma GCC visibility pop
#endif
