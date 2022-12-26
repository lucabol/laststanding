.POSIX:
CC = gcc
WINCC = mingw-gcc

CFLAGS = -Wall -Wextra -pedantic -ffreestanding -nostdlib -static -s -Os -fno-asynchronous-unwind-tables -fno-ident
LDFLAGS = -lgcc # Helps with processors that don't have maths, i.e. float ops https://gcc.gnu.org/onlinedocs/gccint/Libgcc.html

OUTDIR = bin
SRCS = $(wildcard *.c)
BASEALL = $(patsubst %.c,%,$(SRCS))
ALL = $(BASEALL) $(addsuffix .exe, $(BASEALL))
all: $(ALL)

%: %.c lasts.h | $(OUTDIR) Makefile
	$(CC) $(CFLAGS) $< -o $(OUTDIR)/$@ $(LDFLAGS)

# You need kernel32.dll to call kernel functions like ExitProcess, GetCommandLineW etc...
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
