#include <stdio.h>
#include <stdlib.h>
#include "uapi/mosaic.h"
#include "mosaic.h"
#include "volume.h"

static struct migrate *mosaic_copy_start(struct mosaic *m, struct volume *v, int flags)
{
	struct migrate *mig;

	if (!m->m_ops->send_volume_start)
		return NULL;
	if (flags != 0)
		return NULL;

	/* FIXME -- thread safety */
	if (v->mig != NULL)
		return NULL; /* EBUSY */

	mig = malloc(sizeof(*mig));
	v->mig = mig;
	return mig;
}

static void mosaic_copy_stop(struct mosaic *m, struct volume *v)
{
	free(v->mig);
	v->mig = NULL;
}

int mosaic_migrate_vol_send_start(struct volume *v, int fd_to, int flags)
{
	struct mosaic *m = v->m;
	struct migrate *mig;
	int ret;

	mig = mosaic_copy_start(m, v, flags);
	if (!mig)
		return -1;

	mig->sending = true;
	mig->fd = fd_to;

	ret = m->m_ops->send_volume_start(m, v, flags);
	if (ret < 0)
		mosaic_copy_stop(m, v);

	return ret;
}

int mosaic_migrate_vol_send_more(struct volume *v)
{
	struct mosaic *m = v->m;

	if (!v->mig || !v->mig->sending)
		return -1;

	return m->m_ops->send_volume_more(m, v);
}

int mosaic_migrate_vol_recv_start(volume_t v, int fd_from, int flags)
{
	struct mosaic *m = v->m;
	struct migrate *mig;
	int ret;

	mig = mosaic_copy_start(m, v, flags);
	if (!mig)
		return -1;

	mig->sending = false;
	mig->fd = fd_from;

	ret = m->m_ops->recv_volume_start(m, v, flags);
	if (ret < 0)
		mosaic_copy_stop(m, v);

	return ret;
}

int mosaic_migrate_vol_stop(volume_t v)
{
	struct mosaic *m = v->m;

	if (!v->mig)
		return -1;

	m->m_ops->copy_volume_stop(m, v);
	mosaic_copy_stop(m, v);
	return 0;
}
