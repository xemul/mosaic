TESTS = basic
DRIVERS = plain loop btrfs

all: $(DRIVERS)

$(DRIVERS):
	./run.sh ${TESTS} $@

.PHONY: all $(DRIVERS)
