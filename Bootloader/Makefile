ENTRY_FN   = UefiMain
TARGET_DIR = out
SRC_DIR    = src
INC_DIR    = inc
TARGET     = $(TARGET_DIR)/bootx64.efi

CC = x86_64-w64-mingw32-gcc
LD = x86_64-w64-mingw32-ld

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
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(TARGET_DIR)

