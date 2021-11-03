.POSIX:
ifeq ($(OS),Windows_NT)
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

CFLAGS = -ffreestanding -nostdlib -static -Os -lgcc
OUTDIR = bin

%: %.c lasts.h | $(OUTDIR) Makefile
	$(CC) $(CFLAGS) $< -o $(OUTDIR)/$@

%.exe: %.c | $(OUTDIR) Makefile
	$(WINCC) $(CFLAGS) $< -o $(OUTDIR)/$@ -lkernel32

$(OUTDIR):
	mkdir -p $@

clean:
	rm -rf $(OUTDIR)
