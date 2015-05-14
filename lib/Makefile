TGT = libmosaic.so
OBJS =
OBJS += config.o
OBJS += mosaic.o
OBJS += tessera.o
OBJS += overlay.o
OBJS += dmthin.o
OBJS += thin_id.o
OBJS += plain.o
OBJS += ephemeral.o
OBJS += status.o
OBJS += util.o
OBJS += yaml.o

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
