.POSIX:
CC = gcc
WINCC = mingw-gcc

CFLCOM  = -Wall -Wextra -pedantic -static -s -Os -fno-asynchronous-unwind-tables -fno-ident -fno-builtin
CFLAGSF = $(CFLCOM) -ffreestanding -nostdlib
CFLAGSB = $(CFLCOM)
LDFLAGS = -lgcc # Helps with processors that don't have maths, i.e. float ops https://gcc.gnu.org/onlinedocs/gccint/Libgcc.html

OUTDIR = bin
SRCS = $(wildcard *.c)
TARGETS1 = $(patsubst %.c,$(OUTDIR)/f%,$(SRCS))
TARGETS2 = $(patsubst %.c,$(OUTDIR)/b%,$(SRCS))
TARGETS3 = $(patsubst %.c,$(OUTDIR)/f%.exe,$(SRCS))
TARGETS4 = $(patsubst %.c,$(OUTDIR)/b%.exe,$(SRCS))
ALL      = $(TARGETS1) $(TARGETS2) $(TARGETS3) $(TARGETS4)

all: $(ALL)

$(TARGETS1): $(SRCS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGSF) $< -o $@ $(LDFLAGS) -D LSTANDALONE

$(TARGETS2): $(SRCS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGSB) $< -o $@

# You need kernel32.dll to call kernel functions like ExitProcess, GetCommandLineW etc...
$(TARGETS3): $(SRCS)
	@mkdir -p $(OUTDIR)
	$(WINCC) $(CFLAGSF) $< -o $@ $(LDFLAGS) -lkernel32 -D LSTANDALONE

$(TARGETS4): $(SRCS)
	@mkdir -p $(OUTDIR)
	$(WINCC) $(CFLAGSB) $< -o $@

clean:
	rm -rf $(OUTDIR)

check: $(ALL)
	@for f in $^; do echo $$f ; $$f "∮Eda næʃənəl „Anführungszeichen“ 1lI|,0OD,8B γνωρίζω αλημέρα κόσμε, コンニチハ" ; echo ; done

echoes:
	@echo $(SRCS)
	@echo $(TARGETS)

.PHONY: clean all check echoes
