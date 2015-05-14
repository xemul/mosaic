export LD_LIBRARY_PATH=$(pwd)/../lib/
moctl="../moctl/moctl"
yamlck="./chk-yaml.py"

fail() {
	echo "FAIL:" $*
	exit 1
}

MSTDIR="mosaic.status"

creat_st() {
	mkdir "$MSTDIR"
	mkdir "$MSTDIR/rt"
	mkdir "$MSTDIR/thin_map"
	echo > "mosaic.conf"
}

clean_st() {
	umount "$MSTDIR/rt"
	rmdir "$MSTDIR/rt"
	rm -f $MSTDIR/thin_map/*
	rmdir "$MSTDIR/thin_map"
	rm -f "mosaic.conf"
	rmdir "$MSTDIR"
}
