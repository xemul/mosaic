#!/bin/bash

echo "Tests:  $1"
echo "Driver: $2"

for T in ${1//,/ }; do
	. "env.sh"
	. "${T}.sh"
	. "${2}.sh"
done

echo "PASS"
