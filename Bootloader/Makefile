ENTRY_FN   = UefiMain
TARGET_DIR = Output
SRC_DIR    = Source
INC_DIR    = Include
TARGET     = $(TARGET_DIR)/bootx64.efi

CC = x86_64-w64-mingw32-gcc
LD = x86_64-w64-mingw32-ld

MODE ?= -O0 -g -DSN_DEBUG

CFLAGS = \
	-I $(INC_DIR) \
	-std=c17 \
	-mno-red-zone \
	-ffreestanding \
	-nostdlib \
	-Wall \
	-Wextra \
	-Wpedantic

LDFLAGS = \
	--subsystem 10 \
	-e $(ENTRY_FN) \
	-nostdlib

SRC_FILES = $(shell find $(SRC_DIR) -name '*.c')

OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(TARGET_DIR)/%.o,$(SRC_FILES))

.PHONY: all clean

all: $(TARGET)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.c | $(TARGET_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(MODE) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(TARGET_DIR)

