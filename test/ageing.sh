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
umount "$m_dir" || fail "Umount a"

$moctl "tessera" "grow" "t.a" "1" \
		|| fail "T-Grow 1"
$moctl "tessera" "mount" "t.a:1" $m_dir \
		|| fail "T-Mount a:1"
fgrep "V0" "$m_dir/file-a" \
		|| fail "Old data in grow"
echo "V1" > "$m_dir/file-a" \
		|| fail "Update t-data"
umount "$m_dir" || fail "Umount a:1"

$moctl "tessera" "mount" "t.a" $m_dir \
		|| fail "T-Mount a (2)"
fgrep "V0" "$m_dir/file-a" \
		|| fail "Old data in base"
umount "$m_dir" || fail "Umount a (2)"

#FIXME -- not cleaned root-mount is still there
umount "$t_location/a/age-1/root"

echo "* Test mosaic construction out of aged tesserae"

clean
echo "All tests passed"
