#ifndef __MOSAIC_H__
#define __MOSAIC_H__
struct mosaic;
struct volume;
struct vol_map;

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

	int (*new_volume)(struct mosaic *, const char *name, unsigned long size_in_blocks, int make_flags);
	int (*open_volume)(struct mosaic *, struct volume *, int open_flags);

	/*
	 * Clone is optional, NULL means COW-style cloning is not supported.
	 */
	int (*clone_volume)(struct mosaic *, struct volume *from, const char *name, int clone_flags);
	int (*drop_volume)(struct mosaic *, struct volume *, int drop_flags);
	int (*resize_volume)(struct mosaic *, struct volume *, unsigned long size_in_blocks, int resize_flags);

	/*
	 * Mount callback can be optional. In this case ops should
	 * implement the attach_volume callback and library will
	 * mount this device with default fs.
	 */
	int (*mount_volume)(struct mosaic *, struct volume *, const char *path, int mount_flags);
	/*
	 * Umount can be optional, in this case library will just call
	 * umount() and detach_volume (if present).
	 */
	int (*umount_volume)(struct mosaic *, struct volume *, const char *path, int umount_flags);

	/*
	 * Both can be optional, in case raw device access is not
	 * possible for this mosaic type.
	 */
	int (*attach_volume)(struct mosaic *, struct volume *, char *dev, int len, int flags);
	int (*detach_volume)(struct mosaic *, struct volume *);

	int (*get_volume_size)(struct mosaic *, struct volume *, unsigned long *size_in_blocks);

	/*
	 * Migration callbacks. Presence is all or nothing -- first one being
	 * there means migration is supported, otherwise -- no
	 */

	int (*send_volume_start)(struct mosaic *, struct volume *, int flags);
	int (*send_volume_more)(struct mosaic *, struct volume *);
	int (*recv_volume_start)(struct mosaic *, struct volume *, int flags);
	void (*copy_volume_stop)(struct mosaic *, struct volume *);

	/***
	 * Auxiliary ops
	 */

	int (*parse_layout)(struct mosaic *m, char *key, char *val);
};

#define NEW_VOL_WITH_FS	0x1

struct mosaic {
	char *name;
	const struct mosaic_ops *m_ops;
	char *m_loc;
	char *default_fs;
	struct vol_map *vol_map;

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
int mosaic_parse_config(const char *cfg, struct mosaic *);
int bind_vol_loc(struct mosaic *m, struct volume *t, const char *path, int mount_flags);

extern const struct mosaic_ops mosaic_fsimg;
extern const struct mosaic_ops mosaic_btrfs;
extern const struct mosaic_ops mosaic_plain;
extern const struct mosaic_ops mosaic_ploop;

#define MOSAIC_BLOCK_SHIFT	9
#endif
