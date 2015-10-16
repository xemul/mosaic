TESTS = basic clone
DRIVERS = plain fsimg btrfs ploop

all: $(DRIVERS)

$(DRIVERS):
	./run.sh "${TESTS}" $@

clean:
	./run.sh clean

.PHONY: all $(DRIVERS) clean
