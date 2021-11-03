.POSIX:
ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

ifeq ($(detected_OS),Windows)
	CC = gcc
	WINCC = gcc
else
	CC = gcc
	WINCC = ../x86_64-w64-mingw32-cross/bin/x86_64-w64-mingw32-gcc
endif

CFLAGS = -ffreestanding -static -nostdlib -g -lgcc

%: %.c lasts.h
	$(CC) $(CFLAGS) $< -o $@

%.exe: %.c
	$(WINCC) $(CFLAGS) $< -o $@ -lkernel32
