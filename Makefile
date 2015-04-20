OBJS =
OBJS += main.o
OBJS += mosaic.o
OBJS += tessera.o
OBJS += overlay.o
OBJS += config.o
OBJS += status.o

CC = gcc

all: moctl

moctl: $(OBJS)
	$(CC) $^ -o $@ -lyaml

%.o: %.c
	$(CC) $< -o $@ -c
