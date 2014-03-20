CC ?= gcc
CC_WIN ?= i686-w64-mingw32-gcc

CFLAGS ?= -Wall -Wextra -Os

.PHONY: all
all: README.txt README.html rom8x rom8x.exe

rom8x: rom8x.c
	$(CC) $(CFLAGS) -o rom8x rom8x.c
rom8x.exe: rom8x.c
	$(CC_WIN) $(CFLAGS) -o rom8x.exe rom8x.c

README.txt: README.md
	pandoc -s --toc -o $@ README.md

README.html: README.md html/style.css html/nav.js
	pandoc -s --self-contained --toc -N -H html/nav.js --css html/style.css -o $@ README.md
