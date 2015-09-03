echo "*** Testing plain driver"
mkdir plain.dir
echo 'type: plain' > plain.mos
echo 'location: plain.dir' >> plain.mos

run_tests "plain.mos"

rmdir plain.dir
rm -f plain.mos

echo "plain tests PASS"

