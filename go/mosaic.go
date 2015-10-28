package mosaic

// #include <mosaic/mosaic.h>
import "C"
import "syscall"

// Mosaic is a type holding a reference to a mosaic
type Mosaic struct {
	m *C.struct_mosaic
}

// Volume is a type holding a reference to a volume
type Volume struct {
	v *C.struct_volume
}

// Open opens a mosaic
func Open(name string, flags int) (m Mosaic, e error) {
	cname := C.CString(name)
	defer cfree(cname)

	e = nil
	m.m = C.mosaic_open(cname, C.int(flags))
	if m.m == nil {
		e = lastError()
	}

	return
}

// Close closes a mosaic
func (m *Mosaic) Close() {
	C.mosaic_close(m.m)
}

// Mount mounts a mosaic
func (m *Mosaic) Mount(path string, flags int) error {
	cpath := C.CString(path)
	defer cfree(cpath)

	ret := C.mosaic_mount(m.m, cpath, C.int(flags))
	if ret == 0 {
		return nil
	}

	return lastError()
}

// Umount umounts a mosaic
func (m *Mosaic) Umount(path string) error {
	return syscall.Unmount(path, 0)
}
