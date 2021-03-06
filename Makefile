.POSIX:
CC = gcc
WINCC = mingw-gcc

CFLAGS = -Wall -Wextra -pedantic -ffreestanding -nostdlib -static -s -Os -fno-asynchronous-unwind-tables -fno-ident
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

check: all
	@bin/test "∮Eda næʃənəl „Anführungszeichen“ 1lI|,0OD,8B γνωρίζω αλημέρα κόσμε, コンニチハ"
	@bin/test.exe "∮Eda næʃənəl „Anführungszeichen“ 1lI|,0OD,8B γνωρίζω αλημέρα κόσμε, コンニチハ"

echoes:
	echo $(ALL)
	echo $(SRCS)

.PHONY: clean all
