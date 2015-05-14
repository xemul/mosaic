#include "uapi/mosaic.h"
#include "log.h"

mosaic_log_fn print_log;

void mosaic_init_err_log(mosaic_log_fn nlfn)
{
	print_log = nlfn;
}
