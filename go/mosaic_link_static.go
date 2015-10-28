// +build static_build,!test_build

package mosaic

// #include <mosaic/mosaic.h>
// #cgo pkg-config: --static mosaic
// #cgo LDFLAGS: -static
import "C"
