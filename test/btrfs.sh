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
# The umount above fails to stop loopX device:
#  loop: can't delete device /dev/loop3: Device or resource busy
# so as ugly as it looks like, we need to handle this here.
ITER=0
while [ $ITER -lt 3 ]; do
	DEVS=$(losetup -j btrfs.img  | awk -F : '{print $1}')
	test -z "$DEVS" && break
	RET=0
	sleep 0.1
	for D in $DEVS; do
		losetup -d $D; let RET=RET+$?
	done
	[ $RET -eq 0 ] && break
	let ITER++
	echo Retry $ITER...
	sleep 0.5
done

rmdir btrfs.loc
rm -f btrfs.img
rm -f btrfs.mos

echo "btrfs tests PASS"
