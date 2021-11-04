.POSIX:
CC = gcc
WINCC = mingw-gcc

CFLAGS = -ffreestanding -nostdlib -static -s -Os -lgcc
LDFLAGS = -lgcc

OUTDIR = bin
SRCS = $(wildcard *.c)
BASEALL = $(patsubst %.c,%,$(SRCS))
ALL = $(BASEALL) $(addsuffix .exe, $(BASEALL))
all: $(ALL)

%: %.c lasts.h | $(OUTDIR) Makefile
	$(CC) $(CFLAGS) $< -o $(OUTDIR)/$@ $(LDFLAGS)

%.exe: %.c | $(OUTDIR) Makefile
	$(WINCC) $(CFLAGS) $< -o $(OUTDIR)/$@ $(LDFLAGS) -lkernel32

$(OUTDIR):
	mkdir -p $@

clean:
	rm -rf $(OUTDIR)

echoes:
	echo $(ALL)
	echo $(SRCS)

.PHONY: clean all
