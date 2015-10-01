TEST_MOS_ATTACH_DETACH=yes
TEST_MOS_CLONE=yes

echo "*** Testing ploop driver"
mkdir ploop.dir
echo 'type: ploop' > ploop.mos
echo 'location: ploop.dir' >> ploop.mos

run_tests "ploop.mos"

rmdir ploop.dir
rm -f ploop.mos

echo "ploop tests PASS"
