#!/bin/bash

set -x

. env.sh

m_dir="mnt"

creat_st
mkdir $m_dir

$moctl "tessera" "add" "t.a" "ephemeral" "foo:bar" \
		|| fail "Add ephemeral"

$moctl "tessera" "mount" "t.a" "$m_dir" \
		|| fail "Mount ephemeral"

[ -d "$m_dir/foo" ] || fail "No foo dir"
[ -d "$m_dir/bar" ] || fail "No bar dir"
[ -z "$(ls $m_dir | egrep -vw 'foo|bar')" ] || fail "Garbage"

$moctl "tessera" "umount" "t.a" \
		|| fail "Umount"

[ -z "$(ls $m_dir)" ] || fail "Garbage after umount"
rmdir "$m_dir"
clean_st

echo "PASS"
