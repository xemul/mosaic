#ifndef __MOSAIC_H__
#define __MOSAIC_H__
struct mosaic;
struct tessera;

/* Mark for library functions that are not part of public API
 * but are still used by our own tools like moctl.
 */
#define LIB_PROTECTED __attribute__ ((visibility("default")))

struct mosaic_ops {
	const char *name;

	/****
	 * Runtime callbacks
	 */
	int (*init)(struct mosaic *self);
	void (*release)(struct mosaic *self); /* optional */

	/*
	 * Open is called after full config parse
	 */
	int (*open)(struct mosaic *self, int open_flags);
	int (*mount)(struct mosaic *self, const char *path, int mount_flags);

	int (*new_tessera)(struct mosaic *, const char *name, unsigned long size_in_blocks, int make_flags);
	int (*open_tessera)(struct mosaic *, struct tessera *, int open_flags);

	/*
	 * Clone is optional, NULL means COW-style cloning is not supported.
	 */
	int (*clone_tessera)(struct mosaic *, struct tessera *from, const char *name, int clone_flags);
	int (*drop_tessera)(struct mosaic *, struct tessera *, int drop_flags);
	int (*resize_tessera)(struct mosaic *, struct tessera *, unsigned long size_in_blocks, int resize_flags);

	/*
	 * Mount callback can be optional. In this case ops should
	 * implement the attach_tessera callback and library will
	 * mount this device with default fs.
	 */
	int (*mount_tessera)(struct mosaic *, struct tessera *, const char *path, int mount_flags);
	/*
	 * Umount can be optional, in this case library will just call
	 * umount() and detach_tessera (if present).
	 */
	int (*umount_tessera)(struct mosaic *, struct tessera *, const char *path, int umount_flags);

	/*
	 * Both can be optional, in case raw device access is not
	 * possible for this mosaic type.
	 */
	int (*attach_tessera)(struct mosaic *, struct tessera *, char *dev, int len, int flags);
	int (*detach_tessera)(struct mosaic *, struct tessera *, char *dev);

	int (*get_tessera_size)(struct mosaic *, struct tessera *, unsigned long *size_in_blocks);

	/***
	 * Auxiliary ops
	 */

	int (*parse_layout)(struct mosaic *m, char *key, char *val);
};

#define NEW_TESS_WITH_FS	0x1

struct mosaic {
	const struct mosaic_ops *m_ops;
	char *m_loc;
	char *default_fs;

	void *priv;
};

struct mosaic_subdir_priv {
	int m_loc_dir;
	char *fs_subdir;
};

int init_mosaic_subdir(struct mosaic *m);
int open_mosaic_subdir(struct mosaic *m);
void release_mosaic_subdir(struct mosaic *m);
int parse_mosaic_subdir_layout(struct mosaic *m, char *key, char *val);
int bind_mosaic_subdir_loc(struct mosaic *m, const char *path, int mount_flags);

const struct mosaic_ops *mosaic_find_ops(char *type);
LIB_PROTECTED int mosaic_parse_config(const char *cfg, struct mosaic *);
int bind_tess_loc(struct mosaic *m, struct tessera *t, const char *path, int mount_flags);

extern const struct mosaic_ops mosaic_fsimg;
extern const struct mosaic_ops mosaic_btrfs;
extern const struct mosaic_ops mosaic_plain;
extern const struct mosaic_ops mosaic_ploop;

#define MOSAIC_BLOCK_SHIFT	9
#endif
