CC=gcc
CC_WIN=i486-mingw32-gcc

CFLAGS=-Wall -Wextra -Os

.PHONY: all
all: README.txt README.html rom8x rom8x.exe

rom8x: rom8x.c
	$(CC) $(CFLAGS) -o rom8x rom8x.c
rom8x.exe: rom8x.c
	$(CC_WIN) $(CFLAGS) -o rom8x.exe rom8x.c

README.%: README.md
	pandoc -s --toc -o $@ $^
