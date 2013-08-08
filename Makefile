CC=gcc
CC_WIN=i486-mingw-gcc

.PHONY: all
all: README.txt README.html rom8x rom8x.exe

rom8x: rom8x.c
	$(CC) -o rom8x rom8x.c
rom8x.exe: rom8x.c
	$(CC_WIN) -o rom8x.exe rom8x.c

README.%: README.md
	pandoc -s --toc -o $@ $^
