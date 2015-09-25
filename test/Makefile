TESTS = basic
DRIVERS = plain fsimg btrfs ploop

all: $(DRIVERS)

$(DRIVERS):
	./run.sh ${TESTS} $@

.PHONY: all $(DRIVERS)
