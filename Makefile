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

moctl:
	make -C moctl/

clean:
	make -C lib/ clean
	make -C moctl/ clean

.PHONY: all clean $(SUBS)
.DEFAULT_GOAL: all
