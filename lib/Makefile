include ../Makefile.inc

VERSION	     := 0.0
LIBVER_MAJOR := 0
LIBVER_MINOR := 0

LIBMOSAIC		:= libmosaic.a
LIBMOSAIC_SO		:= libmosaic.so
LIBMOSAIC_SO_X		:= $(LIBMOSAIC_SO).$(LIBVER_MAJOR)
LIBMOSAIC_SO_X_Y	:= $(LIBMOSAIC_SO).$(LIBVER_MAJOR).$(LIBVER_MINOR)

PC = mosaic.pc
PCDIR=$(LIBDIR)/pkgconfig

OBJS =
OBJS += mosaic.o
OBJS += volume.o
OBJS += fsimg.o
OBJS += btrfs.o
OBJS += plain.o
OBJS += config.o
OBJS += yaml.o
OBJS += log.o
OBJS += util.o
OBJS += ploop.o ploop_uuid.o
OBJS += volmap.o
OBJS += migrate.o

CFLAGS+= -I../include
CFLAGS+= -fPIC -fvisibility=hidden
LDFLAGS+= -shared -Wl,-soname,$(LIBMOSAIC_SO_X)
LDLIBS+= -lyaml

all: $(LIBMOSAIC_SO) $(PC)
.PHONY: all

$(LIBMOSAIC): $(OBJS)
	ar rcs $@ $+
	ranlib $@

$(LIBMOSAIC_SO_X_Y): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(LIBMOSAIC_SO_X): $(LIBMOSAIC_SO_X_Y)
	ln -sf $^ $@

$(LIBMOSAIC_SO): $(LIBMOSAIC_SO_X)
	ln -sf $^ $@

$(PC): $(PC).in
	sed \
		-e 's|@VERSION@|$(VERSION)|g' \
		-e 's|@LIBDIR@|$(LIBDIR)|g' \
		$^ > $@

install-pc: $(PC)
	install -d $(DESTDIR)$(PCDIR)
	install -m 644 $(PC) $(DESTDIR)$(PCDIR)
.PHONY: install-pc


install: $(LIBMOSAIC_SO) install-pc
	install -d $(DESTDIR)$(LIBDIR)
	install -m 755 $(LIBMOSAIC_SO_X_Y) $(DESTDIR)$(LIBDIR)
	cp -a $(LIBMOSAIC_SO_X) $(LIBMOSAIC_SO) $(DESTDIR)$(LIBDIR)
.PHONY: install

%.d: %.c
	$(CC) $(CFLAGS) -M -MT $@ -MT $(patsubst %.d,%.o,$@) $< -o $@

ifneq ($(MAKECMDGOALS),clean)
-include $(patsubst %.o,%.d,$(OBJS))
endif

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(PC) *.a *.so *.so.* *.d
.PHONY: clean
