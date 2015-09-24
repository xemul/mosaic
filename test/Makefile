TESTS = basic
DRIVERS = plain fsimg btrfs

all: $(DRIVERS)

$(DRIVERS):
	./run.sh ${TESTS} $@

.PHONY: all $(DRIVERS)
