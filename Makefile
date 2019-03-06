# Mazart - Makefile
#
#  Copyright (c) 2019 Alex Dale
#  This project is licensed under the terms of the MIT license.
#  See LICENSE for details.

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

.PHONY: all clean

all: bin/mazart.exe
	@echo "[DONE]"

clean:
	@echo -n "[ RM ] "
	rm -f bin/* obj/*

COMMON_HEADERS = src/common.h

MAZART_OBJS = obj/grid.o obj/deque.o obj/priority.o obj/maze.o obj/color.o obj/maze_image.o obj/config.o obj/colorer.o

obj/colorer.o: src/colorer.c src/colorer.h $(COMMON_HEADERS)
	@echo -n "[OBJ ] "
	$(CC) $(CFLAGS) -c -o obj/colorer.o src/colorer.c

obj/config.o: src/config.c src/config.h $(COMMON_HEADERS)
	@echo -n "[OBJ ] "
	$(CC) $(CFLAGS) -c -o obj/config.o src/config.c

obj/color.o: src/color.c src/color.h $(COMMON_HEADERS)
	@echo -n "[OBJ ] "
	$(CC) $(CFLAGS) -c -o obj/color.o src/color.c

obj/grid.o: src/grid.c src/grid.h $(COMMON_HEADERS)
	@echo -n "[OBJ ] "
	$(CC) $(CFLAGS) -c -o obj/grid.o src/grid.c

obj/deque.o: src/deque.c src/deque.h $(COMMON_HEADERS)
	@echo -n "[OBJ ] "
	$(CC) $(CFLAGS) -c -o obj/deque.o src/deque.c

obj/priority.o: src/priority.c src/priority.h $(COMMON_HEADERS)
	@echo -n "[OBJ ] "
	$(CC) $(CFLAGS) -c -o obj/priority.o src/priority.c

obj/maze.o: src/maze.c src/maze.h $(COMMON_HEADERS)
	@echo -n "[OBJ ] "
	$(CC) $(CFLAGS) -c -o obj/maze.o src/maze.c

obj/maze_image.o: src/maze_image.c src/maze_image.h $(COMMON_HEADERS)
	@echo -n "[OBJ ] "
	$(CC) $(CFLAGS) -c -o obj/maze_image.o src/maze_image.c

bin/mazart.exe: src/main.c $(MAZART_OBJS)
	@echo -n "[ CC ] "
	$(CC) $(CFLAGS) -o bin/mazart.exe src/main.c $(MAZART_OBJS) -lpng -lm

$(shell mkdir -p bin obj)  # Create output directories
