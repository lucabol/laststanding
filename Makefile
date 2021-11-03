CC = gcc
WINCC = x86_64-w64-mingw32-cross/bin/x86_64-w64-mingw32-gcc
CFLAGS = -ffreestanding -static -nostdlib -g -lgcc

%: %.c lasts.h
	$(CC) $(CFLAGS) $< -o $@

%.exe: %.c
	$(CC) $(CFLAGS) $< -o $@ -lkernel32
