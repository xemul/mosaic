ovl_t_location="tess.loc"

tess_type="overlay"

clean_ts() {
	echo "Removing $ovl_t_location"
	rm -rf "$ovl_t_location"
}

prep_ts() {
	echo "Making $ovl_t_location"
	mkdir "$ovl_t_location"
}

new_ts_args() {
	echo "$ovl_t_location/${1}.dir"
}
