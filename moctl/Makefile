include ../Makefile.inc

TGT = moctl
OBJS =
OBJS += main.o
OBJS += create.o

CFLAGS += -I../include

all: $(TGT)

$(TGT): $(OBJS)
	$(CC) -o $@ $^ -lmosaic -L../lib/

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
