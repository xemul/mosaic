#!/bin/bash

set -x

export LD_LIBRARY_PATH=$(pwd)/../lib/
moctl="../moctl/moctl"
yamlck="../chk-yaml.py"
t_location="tess.loc"
tess_type="overlay"

clean() {
	rm -f "mosaic.conf"
	umount "mosaic.status"
	rmdir "mosaic.status"
	rm -rf "$t_location"
}

fail() {
	echo "FAIL:" $*
	exit 1
}

clean

echo "* Test error w/o config file (sanity)"
$moctl && fail "Work without config"
echo "... PASS"

touch "mosaic.conf"
mkdir "mosaic.status"
mkdir "$t_location"

echo "* Check tessera mgmt functionality"
$moctl "tessera" "list" || fail "T-List empty"

$moctl "tessera" "add" "tess.a.n" $tess_type "$t_location/tess.a.dir" || fail "T-Add"

$moctl "tessera" "list" | $yamlck || fail "T-List in yaml"
$moctl "tessera" "list" | fgrep "tess.a.n" || fail "T-List tess.a.n"

$moctl "tessera" "show" "tess.a.n" | $yamlck || fail "T-Show in yaml"
$moctl "tessera" "show" "tess.a.n" | fgrep "tess.a.dir" || fail "T-Show tess.n"

$moctl "tessera" "add" "temp.n" $tess_type "$t_location/x.dir" || fail "T-Add temp"
$moctl "tessera" "del" "temp.n" || fail "T-Del"
$moctl "tessera" "list" | fgrep "temp.n" && fail "T-List deleted"

../chk-yaml.py "mosaic.conf" || fail "Config file not in yaml format"
echo "... PASS"

echo "* Check mosaic mgmt functionality"
$moctl "mosaic" "list" || fail "M-List empty"

$moctl "mosaic" "add" "mos.a.n" || fail "M-Add"

$moctl "mosaic" "list" | $yamlck || fail "M-List in yaml"
$moctl "mosaic" "list" | fgrep "mos.a.n" || fail "M-List mos.a.n"

$moctl "mosaic" "del" "mos.a.n" || fail "M-Del"
$moctl "mosaic" "list" | fgrep "mos.a.n" && fail "M-List deleted"

$moctl "mosaic" "add" "mos.a.n" "tess.a.n" && fail "M-Add bad elem"
$moctl "mosaic" "add" "mos.a.n" "tess.a.n:0:tess.a.loc" || fail "M-Add elem"

$moctl "mosaic" "show" "mos.a.n" | $yamlck || fail "M-Show in yaml"
$moctl "mosaic" "show" "mos.a.n" | fgrep "tess.a.loc" || fail "M-Show w/ elem"

$moctl "mosaic" "change" "mos.a.n" "tess.a.n:0:tess.a.newloc" || fail "M-Change elem"
$moctl "mosaic" "show" "mos.a.n" | fgrep "tess.a.newloc" || fail "M-Show w/ new elem"
$moctl "mosaic" "show" "mos.a.n" | fgrep "tess.a.loc" && fail "M-Show w/o old elem"
$moctl "mosaic" "change" "mos.a.n" "tess.a.n:del" || fail "M-Change del elem"
$moctl "mosaic" "show" "mos.a.n" | fgrep "tess.a.n" && fail "M-Show deleted elem"

clean
echo "All tests passed"
