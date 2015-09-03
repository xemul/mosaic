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

