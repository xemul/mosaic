#include "uapi/mosaic.h"
#include "log.h"
#include "compiler.h"

mosaic_log_fn log_fn;
enum log_level log_level = LOG_WRN;

void mosaic_set_log_fn(mosaic_log_fn lfn)
{
	log_fn = lfn;
}

void mosaic_set_log_lvl(enum log_level l)
{
	log_level = l;
}

static void __maybe_unused test_log_compile(void)
{
	logd("debug %d", 0);
	log("msg %d", 1);
	logw("warning %d", 2);
	loge("error %d", 3);
}
