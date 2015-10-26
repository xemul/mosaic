SUBS =
SUBS += lib
SUBS += moctl

include Makefile.inc

all: $(SUBS)

lib:
	make -C lib/

moctl: lib
	make -C moctl/

install clean:
	make -C lib/ $@
	make -C moctl/ $@
	make -C include/ $@

test: all
	make -C test/

.PHONY: all install clean test $(SUBS)
.DEFAULT_GOAL: all

# include optional local rules
-include Makefile.local
