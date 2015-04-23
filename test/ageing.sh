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

echo "* Check tessera ageing"
$moctl "tessera" "add" "t.a" $tess_type "$t_location/a" \
		|| fail "T-Add a"
$moctl "tessera" "mount" "t.a" $m_dir \
		|| fail "T-Mount a"
echo "V0" > "$m_dir/file-a" \
		|| fail "Touch a"
$moctl "tessera" "umount" "t.a" \
		|| fail "T-Umount a"

$moctl "tessera" "grow" "t.a" "1" \
		|| fail "T-Grow 1"
$moctl "tessera" "mount" "t.a:1" $m_dir \
		|| fail "T-Mount a:1"
fgrep "V0" "$m_dir/file-a" \
		|| fail "Old data in grow"
echo "V1" > "$m_dir/file-a" \
		|| fail "Update t-data"
$moctl "tessera" "umount" "t.a:1" \
		|| fail "Umount a:1"

$moctl "tessera" "mount" "t.a" $m_dir \
		|| fail "T-Mount a (2)"
fgrep "V0" "$m_dir/file-a" \
		|| fail "Old data in base"
$moctl "tessera" "umount" "t.a" \
		|| fail "T-Umount a (2)"

echo "* Test mosaic construction out of aged tesserae"

$moctl "mosaic" "add" "m0" "t.a:0:t" \
		|| fail "M-Add 0"

$moctl "mosaic" "add" "m1" "t.a:1:t" \
		|| fail "M-Add 1"

mkdir "$m_dir/t"

$moctl "mosaic" "mount" "m0" "$m_dir" \
		|| fail "M-Mount 0"

fgrep "V0" "$m_dir/t/file-a" \
		|| fail "Old file in m0"
$moctl "mosaic" "umount" "m0" \
		|| fail "M-Umount 0"

$moctl "mosaic" "mount" "m1" "$m_dir" \
		|| fail "M-Mount 1"
fgrep "V1" "$m_dir/t/file-a" \
		|| fail "New file in m0"
$moctl "mosaic" "umount" "m1" \
		|| fail "M-Umount 1"

rmdir "$m_dir/t"

$moctl "tessera" "del" "t.a" \
		|| fail "T-Del"

clean
echo "All tests passed"
