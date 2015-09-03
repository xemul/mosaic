#!/bin/bash

echo "Tests:  $1"
echo "Driver: $2"

. "env.sh"
. "${1}.sh"
. "${2}.sh"

echo "PASS"
