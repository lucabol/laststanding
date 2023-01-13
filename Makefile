SRC_DIR := test
BIN_DIR := bin

SRC    := $(wildcard $(SRC_DIR)/*.c)
TARGET := $(SRC:$(SRC_DIR)/%.c=$(BIN_DIR)/$(CC)-%)

OPT:=fast

override CPPFLAGS += -I.
override CFLAGS   += -Wall -Wextra -Wpedantic -O$(OPT) -ffreestanding
override LDFLAGS  += -L../libc/bin -nostdlib
override LDLIBS   += -lgcc

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(SRC) | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	@$(RM) -rv $(BIN_DIR)

echo:
	@echo "SRC    : $(SRC)"
	@echo "TARGET : $(TARGET)"

check:
	for x in $(BIN_DIR)/*; do { echo "$$x : "; echo -n "\t"; $$x "t∮Eda næʃənəl „Anführungszeichen“ 1lI|,0OD,8B γνωρίζω αλημέρα κόσμε, コンニチハ"; echo ; } ; done
