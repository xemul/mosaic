#!/bin/bash

set -x

. env.sh

m_dir="mnt/"

clean_ts() {
	echo "No tesserae storage"
}

clean() {
	rmdir "$m_dir"
	clean_st
	clean_ts
}

clean

run_tests() {
	mkdir "$m_dir"
	creat_st
	prep_ts

	echo "* Check tessera mounting"
	t_args=$(new_ts_args "a")
	$moctl "tessera" "add" "t.a" $tess_type $t_args \
			|| fail "T-Add a"

	t_args=$(new_ts_args "b")
	$moctl "tessera" "add" "t.b" $tess_type $t_args \
			|| fail "T-Add b"

	$moctl "tessera" "mount" "t.a" $m_dir \
			|| fail "T-Mount a"

	$moctl "tessera" "del" "t.a" \
			&& fail "T-Del mounted"
	$moctl "tessera" "show" "t.a" | fgrep -A1 "mounted" | fgrep "$m_dir" \
			|| fail "T-Show mounted"

	touch "$m_dir/file-a" || fail "Touch a"
	$moctl "tessera" "umount" "t.a" \
			|| fail "T-Umount a"

	$moctl "tessera" "mount" "t.b" $m_dir \
			|| fail "T-Mount b"

	touch "$m_dir/file-b" || fail "Touch b"
	$moctl "tessera" "umount" "t.b" \
			|| fail "T-Umount b"

	echo "* Check mosaic mounting (based on tesseras)"
	mkdir "$m_dir/mt-a" "$m_dir/mt-b"

	$moctl "mosaic" "add" "m1" "t.a::mt-a" "t.b::mt-b" \
			|| fail "M-Add"
	$moctl "mosaic" "mount" "m1" "$m_dir" \
			|| fail "M-Mount"

	[ -f "$m_dir/mt-a/file-a" ] \
			|| fail "File-a in mosaic"
	[ -f "$m_dir/mt-b/file-b" ] \
			|| fail "File-b in mosaic"

# Mounted mosaic should be undeletable
	$moctl "mosaic" "del" "m1" \
			&& fail "M-Del mounted"

	$moctl "mosaic" "show" "m1" | fgrep -A1 "mounted:" | fgrep "$m_dir" \
			|| fail "M-Show mounted"

	$moctl "mosaic" "umount" "m1" || fail "M-Umount"

	[ -f "$m_dir/mt-a/file-a" ] \
			&& fail "File-a not in mosaic"
	[ -f "$m_dir/mt-b/file-b" ] \
			&& fail "File-b not in mosaic"

	# clean

	rmdir "$m_dir/mt-a" "$m_dir/mt-b"
	$moctl "m" "del" "m1"
	$moctl "t" "del" "t.a"
	$moctl "t" "del" "t.b"

	clean
}

list=${1-"ovl:thin:plain"}

if echo $list | fgrep -q "ovl" ; then
	echo "###### Running tests for overlay"
	. ovl.sh
	run_tests
fi

if echo $list | fgrep -q "thin" ; then
	echo "###### Running tests for thin"
	. thin.sh
	run_tests
fi

if echo $list | fgrep -q "plain" ; then
	echo "###### Running tests for plain"
	. plain.sh
	run_tests
fi

echo "All tests passed"
