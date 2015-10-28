SUBS =
SUBS += lib
SUBS += moctl

include Makefile.inc
NAMEVER=$(NAME)-$(VERSION)$(RELEASE)
TARBALL=$(NAMEVER).tar.xz

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

dist: tar
tar:
	git archive --format tar --prefix '$(NAME)-$(VERSION)/' HEAD \
		| xz -9 > $(TARBALL)

rpms: tar
	rpmbuild -ta $(TARBALL) ${RPMBUILD_ARGS}

.PHONY: all install clean test $(SUBS) dist tar rpms
.DEFAULT_GOAL: all

# include optional local rules
-include Makefile.local
