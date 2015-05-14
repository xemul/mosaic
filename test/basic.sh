#!/bin/bash

set -x

. env.sh

clean_ts() {
	echo "No tesserae storage"
}

clean() {
	clean_st
	clean_ts
}

clean

echo "* Test error w/o config file (sanity)"
$moctl && fail "Work without config"
echo "... PASS"

run_tests() {
	creat_st

	prep_ts

	echo "* Check tessera mgmt functionality"
	$moctl "tessera" "list" \
			|| fail "T-List empty"

	t_args=$(new_ts_args "a")
	$moctl "tessera" "add" "tess.a.n" $tess_type $t_args \
			|| fail "T-Add"

	$moctl "tessera" "list" | $yamlck \
			|| fail "T-List in yaml"
	$moctl "tessera" "list" | fgrep "tess.a.n" \
			|| fail "T-List tess.a.n"

	$moctl "tessera" "show" "tess.a.n" | $yamlck \
			|| fail "T-Show in yaml"

	t_args=$(new_ts_args "temp")
	$moctl "tessera" "add" "temp.n" $tess_type $t_args \
			|| fail "T-Add temp"
	$moctl "tessera" "del" "temp.n" \
			|| fail "T-Del"
	$moctl "tessera" "list" | fgrep "temp.n" \
			&& fail "T-List deleted"

	echo "* Check mosaic mgmt functionality"
	$moctl "mosaic" "list" \
			|| fail "M-List empty"

	$moctl "mosaic" "add" "mos.a.n" \
			|| fail "M-Add"

	$moctl "mosaic" "list" | $yamlck \
			|| fail "M-List in yaml"
	$moctl "mosaic" "list" | fgrep "mos.a.n" \
			|| fail "M-List mos.a.n"

	$moctl "mosaic" "del" "mos.a.n" \
			|| fail "M-Del"
	$moctl "mosaic" "list" | fgrep "mos.a.n" \
			&& fail "M-List deleted"

	$moctl "mosaic" "add" "mos.a.n" "tess.a.n" \
			&& fail "M-Add bad elem"
	$moctl "mosaic" "add" "mos.a.n" "tess.a.n:0:tess.a.loc" \
			|| fail "M-Add elem"

	$moctl "mosaic" "show" "mos.a.n" | $yamlck \
			|| fail "M-Show in yaml"
	$moctl "mosaic" "show" "mos.a.n" | fgrep "tess.a.loc" \
			|| fail "M-Show w/ elem"

# Intermediate check of full config
	$yamlck "mosaic.conf" \
			|| fail "Config file not in yaml format"

	$moctl "mosaic" "change" "mos.a.n" "tess.a.n:0:tess.a.newloc" \
			|| fail "M-Change elem"
	$moctl "mosaic" "show" "mos.a.n" | fgrep "tess.a.newloc" \
			|| fail "M-Show w/ new elem"
	$moctl "mosaic" "show" "mos.a.n" | fgrep "tess.a.loc" \
			&& fail "M-Show w/o old elem"
	$moctl "mosaic" "change" "mos.a.n" "tess.a.n:del" \
			|| fail "M-Change del elem"
	$moctl "mosaic" "show" "mos.a.n" | fgrep "tess.a.n" \
			&& fail "M-Show deleted elem"

	# clean
	$moctl "t" "del" "tess.a.n"

	clean
}

echo "###### Running tests for overlay"
. ovl.sh
run_tests

echo "###### Running tests for thin"
. thin.sh
run_tests

echo "###### Running tests for plain"
. plain.sh
run_tests

echo "All tests passed"
