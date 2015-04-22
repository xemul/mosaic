#!/bin/bash

set -x

export LD_LIBRARY_PATH=$(pwd)/../lib/
moctl="../moctl/moctl"
yamlck="./chk-yaml.py"
t_location="tess.loc"
m_dir="mnt"
tess_type="overlay"

clean() {
	rm -f "mosaic.conf"
	umount "mosaic.status"
	rmdir "mosaic.status"
	rmdir "$m_dir"
	rm -rf "$t_location"
}

fail() {
	echo "FAIL:" $*
	exit 1
}

clean

touch "mosaic.conf"
mkdir "mosaic.status"
mkdir "$t_location"
mkdir "$m_dir"

echo "* Check tessera mounting"
$moctl "tessera" "add" "t.a" $tess_type "$t_location/a" \
		|| fail "T-Add a"
$moctl "tessera" "add" "t.b" $tess_type "$t_location/b" \
		|| fail "T-Add b"

$moctl "tessera" "mount" "t.a" $m_dir \
		|| fail "T-Mount a"

touch "$m_dir/file-a" || fail "Touch a"
umount "$m_dir" || fail "Umount a"

$moctl "tessera" "mount" "t.b" $m_dir \
		|| fail "T-Mount b"

touch "$m_dir/file-b" || fail "Touch b"
umount "$m_dir" || fail "Umount b"

echo "* Check mosaic mounting (based on tesseras)"
mkdir "$m_dir/mt-a" "$m_dir/mt-b"

$moctl "mosaic" "add" "m1" "t.a:0:mt-a" "t.b:0:mt-b" \
		|| fail "M-Add"
$moctl "mosaic" "mount" "m1" "$m_dir" \
		|| fail "M-Mount"

[ -f "$m_dir/mt-a/file-a" ] \
		|| fail "File-a in mosaic"
[ -f "$m_dir/mt-b/file-b" ] \
		|| fail "File-b in mosaic"

$moctl "mosaic" "umount" "m1" || fail "M-Umount"

[ -f "$m_dir/mt-a/file-a" ] \
		&& fail "File-a not in mosaic"
[ -f "$m_dir/mt-b/file-b" ] \
		&& fail "File-b not in mosaic"

rmdir "$m_dir/mt-a" "$m_dir/mt-b"

clean
echo "All tests passed"
