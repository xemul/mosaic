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
