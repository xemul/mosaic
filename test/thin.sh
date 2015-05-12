thin_f_loc="thin/"
thin_f_name="mosaic-test-pool"

tess_type="dm_thin"

clean_ts() {
	echo "Removing dm pool"
	dmsetup remove $thin_f_name
	mt_loop=$(losetup -l -n | awk '/mt.dev/{print $1}')
	dt_loop=$(losetup -l -n | awk '/data.dev/{print $1}')
	losetup -d $mt_loop
	losetup -d $dt_loop
	rm -f "$thin_f_loc/mt.dev"
	rm -f "$thin_f_loc/data.dev"
	rmdir "$thin_f_loc"
	rm -rf "mosaic.thin.map"
}

prep_ts() {
	mkdir "mosaic.thin.map"
	echo "Making dm pool"
	mkdir "$thin_f_loc"
	dd if=/dev/zero of="$thin_f_loc/mt.dev" bs=1024 count=$((32 * 1024))
	dd if=/dev/zero of="$thin_f_loc/data.dev" bs=1024 count=0 seek=$((1024 * 1024))
	losetup -f "$thin_f_loc/mt.dev"
	losetup -f "$thin_f_loc/data.dev"
	mt_loop=$(losetup -l -n | awk '/mt.dev/{print $1}')
	dt_loop=$(losetup -l -n | awk '/data.dev/{print $1}')
	dmsetup create $thin_f_name --table "0 2097152 thin-pool $mt_loop $dt_loop 128 0 0"
}

new_ts_args() {
	echo "/dev/mapper/$thin_f_name" "ext4" "512m"
}

