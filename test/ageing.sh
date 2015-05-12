#!/bin/bash

set -x

. env.sh

m_dir="mnt/"

clean_ts() {
	echo "No tesserae storage"
}

clean() {
	rm -f "mosaic.conf"
	umount "mosaic.status"
	rmdir "mosaic.status"
	rmdir "$m_dir"
	clean_ts
}

clean

run_tests() {
	touch "mosaic.conf"
	mkdir "mosaic.status"
	mkdir "$m_dir"
	prep_ts

	echo "* Check tessera ageing"
	t_args=$(new_ts_args "a")
	$moctl "tessera" "add" "t.a" $tess_type $t_args \
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

	$moctl "tessera" "show" "t.a" > "t.a.show"
	"$yamlck" "t.a.show" || fail "T-Show ages mounted in yaml"
	cat "t.a.show" | fgrep "age: 1" || fail "T-Show age 1"
	cat "t.a.show" | fgrep -A1 "mounted" | fgrep "$m_dir" || fail "T-Show mounted age 1"
	rm -f "t.a.show"

	$moctl "tessera" "umount" "t.a:1" \
			|| fail "Umount a:1"

	$moctl "tessera" "mount" "t.a" $m_dir "ro" \
			|| fail "T-Mount a (2)"
	fgrep "V0" "$m_dir/file-a" \
			|| fail "Old data in base"
	$moctl "tessera" "umount" "t.a" \
			|| fail "T-Umount a (2)"

	echo "* Test mosaic construction out of aged tesserae"

	$moctl "mosaic" "add" "m0" "t.a:0:t:options=ro" \
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
}

echo "###### Running tests for overlay"
. ovl.sh
run_tests

echo "###### Running tests for thin"
. thin.sh
run_tests

clean
echo "All tests passed"
