include ../Makefile.inc

TGT = moctl
OBJS =
OBJS += main.o
OBJS += create.o

CFLAGS += -I../include

all: $(TGT)
.PHONY: all

$(TGT): $(OBJS)
	$(CC) -o $@ $^ -lmosaic -L../lib/

%.d: %.c
	$(CC) $(CFLAGS) -M -MT $@ -MT $(patsubst %.d,%.o,$@) $< -o $@

ifneq ($(MAKECMDGOAL),clean)
-include $(patsubst %.o,%.d,$(OBJS))
endif

%.o: %.c
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

install: $(TGT)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TGT) $(DESTDIR)$(BINDIR)
.PHONY: install

clean:
	rm -rf $(TGT) $(OBJS) *.d
.PHONY: clean
