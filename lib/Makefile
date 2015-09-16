LIBVER_MAJOR := 0
LIBVER_MINOR := 0

LIBMOSAIC		:= libmosaic.a
LIBMOSAIC_SO		:= libmosaic.so
LIBMOSAIC_SO_X		:= $(LIBMOSAIC_SO).$(LIBVER_MAJOR)
LIBMOSAIC_SO_X_Y	:= $(LIBMOSAIC_SO).$(LIBVER_MAJOR).$(LIBVER_MINOR)

OBJS =
OBJS += mosaic.o
OBJS += tessera.o
OBJS += fsimg.o
OBJS += btrfs.o
OBJS += plain.o
OBJS += config.o
OBJS += yaml.o
OBJS += log.o
OBJS += util.o

CFLAGS+= -fPIC -fvisibility=hidden
LDFLAGS+= -shared -Wl,-soname,$(LIBMOSAIC_SO_X)
LDLIBS+= -lyaml

all: $(LIBMOSAIC_SO)
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

%.d: %.c
	$(CC) $(CFLAGS) -M -MT $@ -MT $(patsubst %.d,%.o,$@) $< -o $@

ifneq ($(MAKECMDGOAL),clean)
-include $(patsubst %.o,%.d,$(OBJS))
endif

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) *.a *.so *.so.* *.d
.PHONY: clean
