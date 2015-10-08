include ../Makefile.inc

LIB_PATH=../lib

BIN = moctl
REALBIN = .$(BIN)-real
OBJS =
OBJS += main.o
OBJS += create.o

CFLAGS += -fPIE -I../include

all: $(BIN)
.PHONY: all

$(BIN): wrapper.sh.in $(REALBIN)
	sed \
		 -e 's|@REALBIN@|$(REALBIN)|g' \
		 -e 's|@LIB_PATH@|$(LIB_PATH)|g' \
		< $< > $@ \
	&& chmod a+x $@

$(REALBIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lmosaic -L $(LIB_PATH)

%.d: %.c
	$(CC) $(CFLAGS) -M -MT $@ -MT $(patsubst %.d,%.o,$@) $< -o $@

ifneq ($(MAKECMDGOALS),clean)
-include $(patsubst %.o,%.d,$(OBJS))
endif

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(REALBIN)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(REALBIN) $(DESTDIR)$(BINDIR)/$(BIN)
.PHONY: install

clean:
	rm -rf $(BIN) $(REALBIN) $(OBJS) *.d
.PHONY: clean
