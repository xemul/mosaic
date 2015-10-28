// +build test_build

package mosaic

// Compile and link against uninstalled library and headers,
// residing in ..
// Note you still need to pass LD_LIBRARY_PATH=../lib when running.

// #include <mosaic.h>
// #cgo CFLAGS: -I${SRCDIR}/../include/uapi/
// #cgo LDFLAGS: -L${SRCDIR}/../lib -lmosaic
import "C"
