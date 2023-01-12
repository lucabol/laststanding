.POSIX:
CC = gcc
WINCC = mingw-gcc

CFLCOM  = -Wall -Wextra -pedantic -static -s -Os -fno-asynchronous-unwind-tables -fno-ident -fno-builtin
CFLAGSF = $(CFLCOM) -ffreestanding -nostdlib
LDFLAGS = -lgcc # Helps with processors that don't have maths, i.e. float ops https://gcc.gnu.org/onlinedocs/gccint/Libgcc.html

OUTDIR 	 = bin
SRCS 		 = $(wildcard *.c)
INCL		 = $(wildcard *.h)
TARGETS1 = $(patsubst %.c,$(OUTDIR)/%,$(SRCS))
TARGETS2 = $(patsubst %.c,$(OUTDIR)/%.exe,$(SRCS))
ALL      = $(TARGETS1) $(TARGETS2)

all: $(ALL)

$(TARGETS1): $(SRCS) $(INCL)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGSF) $< -o $@ $(LDFLAGS) -D LSTANDALONE

# You need kernel32.dll to call kernel functions like ExitProcess, GetCommandLineW etc...
$(TARGETS2): $(SRCS) $(INCL)
	@mkdir -p $(OUTDIR)
	$(WINCC) $(CFLAGSF) $< -o $@ $(LDFLAGS) -lkernel32 -D LSTANDALONE

clean:
	rm -rf $(OUTDIR)

check: $(ALL)
	@echo
	@for f in $^; do echo $$f ; $$f "∮Eda næʃənəl „Anführungszeichen“ 1lI|,0OD,8B γνωρίζω αλημέρα κόσμε, コンニチハ" ; echo ; done

echoes:
	@echo $(SRCS)
	@echo $(TARGETS)

.PHONY: clean all check echoes
