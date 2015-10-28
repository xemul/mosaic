package mosaic

// Auxiliary helpers to simplify life with CGo

// #include <stdlib.h>
import "C"
import "unsafe"

// cfree frees a C string
func cfree(c *C.char) {
	C.free(unsafe.Pointer(c))
}
