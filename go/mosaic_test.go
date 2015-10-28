package mosaic

// A test suite

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"os"
	"testing"
)

var (
	oldPwd  string
	testDir string
	drivers []string
)

func init() {
	drivers = []string{"plain", "fsimg", "btrfs", "ploop"}
}

// abort is used when the tests after the current one
// can't be run as one of their prerequisite(s) failed
func abort(format string, args ...interface{}) {
	s := fmt.Sprintf("ABORT: "+format+"\n", args...)
	f := bufio.NewWriter(os.Stderr)
	f.Write([]byte(s))
	f.Flush()
	cleanup()
	os.Exit(1)
}

// Check for a fatal error, call abort() if it is
func chk(err error) {
	if err != nil {
		abort("%s", err)
	}
}

func prepare(dir string) {
	var err error

	oldPwd, err = os.Getwd()
	chk(err)

	testDir, err = ioutil.TempDir(oldPwd, dir)
	chk(err)

	err = os.Chdir(testDir)
	chk(err)
}

func cleanup() {
	if oldPwd != "" {
		os.Chdir(oldPwd)
	}
	if testDir != "" {
		os.RemoveAll(testDir)
	}
}

func testMosaicMountUmount(t *testing.T, drv string) {
	prepare("tmp-test-" + drv)

	t.Logf("Mosaic mount/umount test for %s", drv)
	dir := drv + ".dir"
	err := os.Mkdir(dir, 0755)
	chk(err)

	mosfile := "./" + drv + ".mos"
	contents := []byte("type: " + drv + "\nlocation: " + dir + "\n")
	err = ioutil.WriteFile(mosfile, contents, 0644)
	chk(err)

	mntdir := "mmnt"
	err = os.Mkdir(mntdir, 0755)
	chk(err)

	m, err := Open(mosfile, 0)
	chk(err)

	err = m.Mount(mntdir, 0)
	chk(err)

	tmpfile := mntdir + "/tfile"
	err = ioutil.WriteFile(tmpfile, []byte("test"), 0644)
	chk(err)

	err = m.Umount(mntdir)
	chk(err)

	_, err = ioutil.ReadFile(tmpfile)
	// expecting ENOENT
	if err == nil || !os.IsNotExist(err) {
		t.Fatalf("Unexpectedly found %s: %s", tmpfile, err)
	}

	err = m.Mount(mntdir, 0)
	chk(err)

	err = os.Remove(tmpfile)
	chk(err)

	err = m.Umount(mntdir)
	chk(err)

	cleanup()
}

func TestMosaicMountUmount(t *testing.T) {
	for _, drv := range drivers {
		testMosaicMountUmount(t, drv)
	}
}
