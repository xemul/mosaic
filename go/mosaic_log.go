package mosaic

// Implement logging for mosaic library in a way so that
// the normal log messages go to stdout, errors to stderr,
// plus we save the last error to a string buffer, to be
// returned by the lastError() function.

/*
#include <stdarg.h>
#include <stdio.h>
#include "../include/uapi/mosaic.h"

#define BUFSIZE 4096
// thread local storage
static __thread char errbuf[BUFSIZE];

static char *buf(void)
{
	return errbuf;
}

// A logger function suitable for mosaic_set_log_fn.
// I seriously doubt it can be implemented in Go.
static void logger(int level, const char *f, ...)
{
	va_list args;

	va_start(args, f);
	if (level <= LOG_WRN) {
		char *b = buf();
		vsnprintf(b, BUFSIZE, f, args);
		fputs(b, stderr);
	}
	else {
		vprintf(f, args);
	}
	va_end(args);
}

static void init_log(void)
{
	mosaic_set_log_fn(logger);
	// FIXME: debug logging for now
	mosaic_set_log_lvl(LOG_DBG);
}
*/
import "C"
import "errors"
import "strings"

func init() {
	C.init_log()
}

func lastError() error {
	return errors.New(strings.TrimRight(C.GoString(C.buf()), "\n"))
}
