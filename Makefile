SUBS =
SUBS += lib
SUBS += moctl

DIR = $(shell pwd)
CFLAGS = -I$(DIR)/include/ -Wall -Werror
CC = gcc

export CFLAGS
export CC

all: $(SUBS)

lib:
	make -C lib/

moctl: lib
	make -C moctl/

clean:
	make -C lib/ clean
	make -C moctl/ clean

test: all
	make -C test/

.PHONY: all clean $(SUBS)
.DEFAULT_GOAL: all
