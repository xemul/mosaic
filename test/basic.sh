#!/bin/bash

export LD_LIBRARY_PATH="$(pwd)/../lib/"
moctl="../moctl/moctl"
set -x

function fail()
{
	echo "FAIL:" $@
	exit 1
}

function run_tests()
{
	local mname=$1

	mkdir mmnt
	mkdir tmnt

	echo "* Testing mosaic FS"
	$moctl $mname mount - mmnt - || fail "Can't mount mosaic (1)"
	echo "test" > mmnt/tfile
	$moctl $mname umount - mmnt || fail "Can't umount mosaic"
	[ -f mmnt/tfile ] && fail "Unexpected file"
	$moctl $mname mount - mmnt - || fail "Can't mount mosaic (2)"
	fgrep -q "test" mmnt/tfile || fail "File not preserved in mosaic"

	echo "* Cleaning mosaic FS"
	rm -f mmnt/tfile
	$moctl $mname umount - mmnt || fail "Can't clean mosaic"

	echo "* Testing tesserae"
	$moctl $mname new fs test_fs 128m || fail "Can't create fs"
	$moctl $mname mount test_fs tmnt - || fail "Can't mount fs (1)"
	echo "t-test" > tmnt/t-tfile
	$moctl $mname umount test_fs tmnt || fail "Can't umount fs"
	[ -f tmnt/t-tfile ] && fail "Unexpected file"
	$moctl $mname mount test_fs tmnt - || fail "Can't mount fs (2)"
	fgrep -q "t-test" tmnt/t-tfile || fail "File not preserved in fs"

	echo "* Cleaning up tessera"
	$moctl $mname umount test_fs tmnt || fail "Can't clean fs"
	$moctl $mname drop test_fs || fail "Can't drop tessera"

	rmdir mmnt
	rmdir tmnt
}

# echo "*** Testing fsimg driver"
# mkdir fsimg.loc
# echo 'type: fsimg' > fsimg.mos
# echo 'location: fsimg.loc' >> fsimg.mos
# run_tests "fsimg.mos"
# rmdir fsimg.loc
# rm -f fsimg.mos
# 
# echo "fsimg tests PASS"

echo "*** Testing btrfs driver (dir case)"
touch btrfs.img
truncate --size $((64 * 1024 * 1024)) btrfs.img
mkfs -t btrfs btrfs.img || fail "Can't make btrfs FS"
mkdir btrfs.loc
mount -t btrfs btrfs.img btrfs.loc -o loop || fail "Can't mount btrfs FS"
echo 'type: btrfs' > btrfs.mos
echo 'location: btrfs.loc' >> btrfs.mos
run_tests "btrfs.mos"
umount btrfs.loc
rmdir btrfs.loc
rm -f btrfs.img
rm -f btrfs.mos

echo "btrfs tests PASS"
