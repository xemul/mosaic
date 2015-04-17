OBJS =
OBJS += main.o
OBJS += config.o

CC = gcc

all: mosaic

mosaic: $(OBJS)
	$(CC) $^ -o $@ -lyaml

%.o: %.c
	$(CC) $< -o $@ -c
