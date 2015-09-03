echo "*** Testing fsimg driver"
mkdir fsimg.loc
echo 'type: fsimg' > fsimg.mos
echo 'location: fsimg.loc' >> fsimg.mos

run_tests "fsimg.mos"

rmdir fsimg.loc
rm -f fsimg.mos

echo "fsimg tests PASS"
