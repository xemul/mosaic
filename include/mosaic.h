#ifndef __MOSAIC_H__
#define __MOSAIC_H__
struct mosaic;
struct tessera;

struct mosaic_ops {
	int (*open)(struct mosaic *self, int open_flags);
	void (*release)(struct mosaic *self); /* optional */
	int (*mount)(struct mosaic *self, const char *path, int mount_flags);

	int (*new_tessera)(struct mosaic *, char *name, unsigned long size_in_blocks, char *fsname, int make_flags);
	int (*open_tessera)(struct mosaic *, struct tessera *, int open_flags);
	int (*clone_tessera)(struct mosaic *, struct tessera *from, char *name, int clone_flags); /* optional */
	int (*drop_tessera)(struct mosaic *, struct tessera *, int drop_flags);
	int (*mount_tessera)(struct mosaic *, struct tessera *, char *path, int mount_flags);
	int (*umount_tessera)(struct mosaic *, struct tessera *, char *path, int umount_flags);
	int (*attach_tessera)(struct mosaic *, struct tessera *, char *devs, int len, int flags);
	int (*detach_tessera)(struct mosaic *, struct tessera *, char *devs);
	int (*resize_tessera)(struct mosaic *, struct tessera *, unsigned long size_in_blocks, int resize_flags);
};

struct mosaic {
	const struct mosaic_ops *m_ops;
	char *m_loc;
	char *default_fs;

	void *priv;
};

const struct mosaic_ops *mosaic_find_ops(char *type);
int mosaic_parse_config(const char *cfg, struct mosaic *);

extern const struct mosaic_ops mosaic_fsimg;

#define MOSAIC_BLOCK_SHIFT	9
#endif
