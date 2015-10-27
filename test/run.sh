#!/bin/bash

set -e
set -u
set -o pipefail

TDIR=test-rm-me-XXXXXXXX
if [ "$1" = "clean" ]; then
	rm -rf ${TDIR//X/?}
	exit
fi

TESTDIR=$(mktemp -d -p . $TDIR)
echo "Tests:  $1"
echo "Driver: $2"
echo "Dir:    $TESTDIR"

cd $TESTDIR
for T in ${1//,/ }; do
	. ../"env.sh"
	. ../"${T}.sh"
	. ../"${2}.sh"
done

cd ..
rm -rf $TESTDIR
echo "PASS"
