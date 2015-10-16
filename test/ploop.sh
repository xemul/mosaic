# Check if ploop is available
if test -f /proc/vz/ploop_minor && ploop list >/dev/null; then
	true # ploop seems to be working
else
	echo "* ploop not available, skipping tests"
	return 0
fi

echo "*** Testing ploop driver"
mkdir ploop.dir
cat > ploop.mos << EOF
type: ploop
location: ploop.dir
volumeMap: \\([a-z]\\)\\([a-z][a-z]\\)\\(.*\\) \\1/\\2/\\3
EOF

run_tests "./ploop.mos"

rmdir ploop.dir || fail "mosaic dir not empty"
rm -f ploop.mos

echo "ploop tests PASS"
