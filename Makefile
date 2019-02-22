# Maze-Art - Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c17

.PHONY: all clean

all: bin/mazart.exe
	@echo "[DONE]"

clean:
	@echo -n "[ RM ] "
	rm -f bin/* obj/*

COMMON_HEADERS = src/common.h

MAZART_OBJS = obj/grid.o

obj/grid.o: src/grid.c src/grid.h $(COMMON_HEADERS)
	@echo -n "[OBJ ]"
	$(CC) $(CFLAGS) -c -o obj/grid.o src/grid.c

bin/mazart.exe: src/main.c $(MAZART_OBJS)
	@echo -n "[ CC ]"
	$(CC) $(CFLAGS) -o bin/mazart.exe src/main.c $(MAZART_OBJS)
