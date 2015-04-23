export LD_LIBRARY_PATH=$(pwd)/../lib/
moctl="../moctl/moctl"
yamlck="./chk-yaml.py"

fail() {
	echo "FAIL:" $*
	exit 1
}
