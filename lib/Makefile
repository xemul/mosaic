TGT = libmosaic.so
OBJS =
OBJS += config.o
OBJS += mosaic.o
OBJS += tessera.o
OBJS += overlay.o
OBJS += plain.o
OBJS += status.o

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
