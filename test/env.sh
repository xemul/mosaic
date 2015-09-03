export LD_LIBRARY_PATH="$(pwd)/../lib/"
moctl="../moctl/moctl"
set -x

function fail()
{
	echo "FAIL:" $@
	exit 1
}
