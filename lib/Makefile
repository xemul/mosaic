TGT = libmosaic.so
OBJS =
OBJS += mosaic.o
OBJS += tessera.o
OBJS += fsimg.o
OBJS += btrfs.o
OBJS += config.o
OBJS += yaml.o
OBJS += log.o
OBJS += util.o

all: $(TGT)

$(TGT): $(OBJS)
	$(CC) -shared -lyaml -o $@ $^

%.d: %.c
	$(CC) $(CFLAGS) -M -MT $@ -MT $(patsubst %.d,%.o,$@) $< -o $@

ifneq ($(MAKECMDGOAL),clean)
-include $(patsubst %.o,%.d,$(OBJS))
endif

%.o: %.c
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

clean:
	rm -rf $(TGT) $(OBJS) *.d

.PHONY: all clean
