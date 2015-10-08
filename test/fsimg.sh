TEST_MOS_ATTACH_DETACH=yes

echo "*** Testing fsimg driver"
mkdir fsimg.loc
cat > fsimg.mos << EOF
type: fsimg
location: fsimg.loc
volumeMap: \\([a-z]\\)\\([^_]*\\)_\\(.*\\) \\1/\\2/\\3
EOF

run_tests "fsimg.mos"

rmdir fsimg.loc || fail "mosaic dir not empty"
rm -f fsimg.mos

echo "fsimg tests PASS"
