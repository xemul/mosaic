moctl="../moctl/moctl"
set -x

function fail()
{
	echo "FAIL:" $@
	exit 1
}
