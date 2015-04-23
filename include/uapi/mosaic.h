#ifndef __MOSAIC_UAPI_H__
#define __MOSAIC_UAPI_H__
/*
 * Mosaics
 */
struct mosaic;
struct element;

int mosaic_iterate(int (*cb)(struct mosaic *, void *), void *);
struct mosaic *mosaic_find_by_name(char *name);
int mosaic_iterate_elements(struct mosaic *, int (*cb)(struct mosaic *, struct element *, void *), void *);
int mosaic_iterate_mounted(struct mosaic *, int (*cb)(struct mosaic *, char *mp, void *), void *);

struct mosaic *mosaic_new(char *name);
int mosaic_set_element(struct mosaic *m, char *name, int age, char *at, char *opt);
int mosaic_del_element(struct mosaic *m, char *name);
int mosaic_add(struct mosaic *m);
int mosaic_update(struct mosaic *m);
int mosaic_del(struct mosaic *m);

int mosaic_mount(struct mosaic *m, char *mountpoint, char *options);
int mosaic_umount(struct mosaic *m, char *mountpoint);

/*
 * Tesserae
 */
struct tessera;

int mosaic_iterate_tesserae(int (*cb)(struct tessera *, void *), void *x);
struct tessera *mosaic_find_tessera(char *name);
int mosaic_add_tessera(char *type, char *name, int n_opts, char **opts);
int mosaic_del_tessera(struct tessera *t);
int mosaic_mount_tessera(struct tessera *t, int age, char *at, char *options);
int mosaic_umount_tessera(struct tessera *t, int age, char *at);
int mosaic_grow_tessera(struct tessera *t, int age, int from_age);


/*
 * Config
 */
int mosaic_load_config(void);
#endif
