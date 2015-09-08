#ifndef __MOSAIC_H__
#define __MOSAIC_H__
struct mosaic;
struct tessera;

struct mosaic_ops {
	int (*open)(struct mosaic *self, int open_flags);
	void (*release)(struct mosaic *self); /* optional */
	int (*mount)(struct mosaic *self, const char *path, int mount_flags);

	int (*new_tessera)(struct mosaic *, char *name, unsigned long size_in_blocks, int make_flags);
	int (*open_tessera)(struct mosaic *, struct tessera *, int open_flags);

	/*
	 * Clone is optional, NULL means COW-style cloning is not supported.
	 */
	int (*clone_tessera)(struct mosaic *, struct tessera *from, char *name, int clone_flags);
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
	int (*umount_tessera)(struct mosaic *, struct tessera *, char *path, int umount_flags);

	/*
	 * Both can be optional, in case raw device access is not
	 * possible for this mosaic type.
	 */
	int (*attach_tessera)(struct mosaic *, struct tessera *, char *devs, int len, int flags);
	int (*detach_tessera)(struct mosaic *, struct tessera *, char *devs);

	int (*get_tessera_size)(struct mosaic *, struct tessera *, unsigned long *size_in_blocks);
};

#define NEW_TESS_WITH_FS	0x1

struct mosaic {
	const struct mosaic_ops *m_ops;
	char *m_loc;
	char *default_fs;
	char *layout;

	void *priv;
};

struct mosaic_subdir_priv {
	int dir;
};

int open_mosaic_subdir(struct mosaic *m);
void release_mosaic_subdir(struct mosaic *m);

const struct mosaic_ops *mosaic_find_ops(char *type);
int mosaic_parse_config(const char *cfg, struct mosaic *);
int bind_mosaic_loc(struct mosaic *m, const char *path, int mount_flags);
int bind_tess_loc(struct mosaic *m, struct tessera *t, const char *path, int mount_flags);

extern const struct mosaic_ops mosaic_fsimg;
extern const struct mosaic_ops mosaic_btrfs;
extern const struct mosaic_ops mosaic_plain;

#define MOSAIC_BLOCK_SHIFT	9
#endif
