plain_t_location="tess.loc"

tess_type="plain"

clean_ts() {
	echo "Removing $plain_t_location"
	rm -rf "$plain_t_location"
}

prep_ts() {
	echo "Making $plain_t_location"
	mkdir "$plain_t_location"
}

new_ts_args() {
	echo "$plain_t_location/${1}.dir"
}

