TGT = libmosaic.so
OBJS =
OBJS += mosaic.o
OBJS += fsimg.o
OBJS += config.o
OBJS += yaml.o
OBJS += log.o

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
