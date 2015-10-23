function run_tests()
{
	local mname=$1

	mkdir mmnt
	mkdir tmnt

	echo "* Testing mosaic FS"
	$moctl $mname mount - mmnt || fail "Can't mount mosaic (1)"
	echo "test" > mmnt/tfile
	$moctl $mname umount - mmnt || fail "Can't umount mosaic"
	[ -f mmnt/tfile ] && fail "Unexpected file"
	$moctl $mname mount - mmnt || fail "Can't mount mosaic (2)"
	fgrep -q "test" mmnt/tfile || fail "File not preserved in mosaic"

	echo "* Cleaning mosaic FS"
	rm -f mmnt/tfile
	$moctl $mname umount - mmnt || fail "Can't clean mosaic"

	echo "* Testing volumes"
	$moctl $mname have test_fs | fgrep -q ' exists: no' \
		|| fail "existense test (1) failed"
	$moctl $mname new fs test_fs 512m || fail "Can't create fs"
	$moctl $mname have test_fs | fgrep -q ' exists: yes' \
		|| fail "existense test (2) failed"

	if $moctl $mname info | fgrep -q 'features' | fgrep -q 'bdev'; then
		$moctl $mname attach test_fs \
			|| fail "Can't attach device"
		$moctl $mname attach test_fs \
			&& fail "Unexpected success of attach (2)"
		$moctl $mname detach test_fs \
			|| fail "Can't detach device"
		$moctl $mname detach test_fs \
			&& fail "Unexpected success of detach (2)"
	fi

	$moctl $mname mount test_fs tmnt || fail "Can't mount fs (1)"
	echo "t-test" > tmnt/t-tfile
	$moctl $mname umount test_fs tmnt || fail "Can't umount fs"
	[ -f tmnt/t-tfile ] && fail "Unexpected file"
	$moctl $mname mount test_fs tmnt || fail "Can't mount fs (2)"
	fgrep -q "t-test" tmnt/t-tfile || fail "File not preserved in fs"

	echo "* Cleaning up volume"
	$moctl $mname umount test_fs tmnt || fail "Can't clean fs"
	$moctl $mname drop test_fs || fail "Can't drop volume"

	rmdir mmnt
	rmdir tmnt
}

