#ifndef __MOSAIC_H__
#define __MOSAIC_H__
struct mosaic;

struct mosaic_ops {
	int (*open)(struct mosaic *self, int open_flags);
	void (*release)(struct mosaic *self);
	int (*mount)(struct mosaic *self, const char *path, int mount_flags);
};

struct mosaic {
	const struct mosaic_ops *m_ops;
	char *m_loc;
	char *default_fs;
};

const struct mosaic_ops *mosaic_find_ops(char *type);
int mosaic_parse_config(const char *cfg, struct mosaic *);

extern const struct mosaic_ops mosaic_fsimg;
#endif
