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
