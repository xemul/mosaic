function run_tests()
{
	local mname=$1

	if ! $moctl $mname info | grep '^features:' | fgrep -qw 'clone'; then
		echo "* skipping clone test"
		return 0
	fi

	mkdir tmnt tmnt1 tmnt2 tmnt21

	echo "* Testing cloning"
	$moctl $mname create fs orig 512m || fail "Can't create orig fs"
	$moctl $mname mount orig tmnt || fail "Can't mount orig"
	echo "O is for OpenVZ" > tmnt/t-tfile
	$moctl $mname umount orig tmnt || fail "Can't umount orig"

	$moctl $mname clone orig clone1 || fail "Can't clone (1)"
	$moctl $mname mount orig tmnt || fail "Can't mount orig"
	$moctl $mname clone orig clone2 || fail "Can't clone (2)"

	$moctl $mname mount clone1 tmnt1 || fail "Can't mount clone1"
	$moctl $mname mount clone2 tmnt2 || fail "Can't mount clone2"
	diff -rNq tmnt tmnt1 || fail "clone1 not identical to orig"
	diff -rNq tmnt tmnt2 || fail "clone2 not identical to orig"
	diff -rNq tmnt1 tmnt2 || fail "clone1 not identical to clone2"

	echo "C is for CRIU" > tmnt2/t-tfile2
	$moctl $mname clone clone2 clone21 || fail "Can't clone (2.1)"
	$moctl $mname mount clone21 tmnt21 || fail "Can't mount clone21"
	diff -rNq tmnt2 tmnt21 || fail "clone21 not identical to clone2"

	$moctl $mname umount orig tmnt || fail "Can't umount orig"
	$moctl $mname drop orig || fail "Can't drop orig"

	$moctl $mname umount clone1 tmnt1 || fail "Can't umount clone1"
	$moctl $mname drop clone1 || fail "Can't drop clone1"

	$moctl $mname umount clone2 tmnt2 || fail "Can't umount clone2"
	$moctl $mname umount clone21 tmnt21 || fail "Can't umount clone21"

	$moctl $mname mount clone2 tmnt2 || fail "Can't mount clone2"
	$moctl $mname mount clone21 tmnt21 || fail "Can't mount clone21"

	diff -rNq tmnt2 tmnt21 || fail "clone2 not identical to clone21"

	$moctl $mname umount clone2 tmnt2 || fail "Can't umount clone2"
	$moctl $mname umount clone21 tmnt21 || fail "Can't umount clone21"

	$moctl $mname drop clone2 || fail "Can't drop clone2"
	$moctl $mname drop clone21 || fail "Can't drop clone21"

	rmdir tmnt tmnt1 tmnt2 tmnt21
}
