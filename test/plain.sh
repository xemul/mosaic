echo "*** Testing plain driver"
mkdir plain.dir
cat > plain.mos << EOF
type: plain
location: plain.dir
volumeMap: \\([a-z]\\)\\([^_]*\\)_\\(.*\\) \\1/\\2/\\3
EOF

run_tests "./plain.mos"

rmdir plain.dir || fail "mosaic dir not empty"
rm -f plain.mos

echo "plain tests PASS"

