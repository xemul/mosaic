OBJS =
OBJS += main.o
OBJS += config.o
OBJS += mosaic.o
OBJS += tessera.o

CC = gcc

all: moctl

moctl: $(OBJS)
	$(CC) $^ -o $@ -lyaml

%.o: %.c
	$(CC) $< -o $@ -c
