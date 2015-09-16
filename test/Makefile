TESTS = basic

all: plain loop btrfs

loop:
	./run.sh ${TESTS} fsimg

plain:
	./run.sh ${TESTS} plain

btrfs:
	./run.sh ${TESTS} btrfs

.PHONY: all loop plain btrfs
