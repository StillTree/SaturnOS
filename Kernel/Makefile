TARGET_DIR = out
SRC_DIR    = src
INC_DIR    = inc
TARGET     = $(TARGET_DIR)/Kernel.elf

CC = x86_64-elf-gcc
AS  = x86_64-elf-as
LD  = x86_64-elf-ld

MODE ?= -O0 -g -DSK_DEBUG

CCFLAGS = \
	-I $(INC_DIR) \
	-std=c23 \
	-mno-red-zone \
	-mno-mmx \
	-mcmodel=kernel \
	-mno-sse \
	-mno-sse2 \
	-ffreestanding \
	-nostdlib \
	-Wall \
	-Wextra \
	-Wpedantic

INTERRUPT_CCFLAGS = \
	-mgeneral-regs-only

LDFLAGS = \
	-T linker.ld \
	-static \
	-z max-page-size=0x1000 \
	-nostdlib

C_FILES           = $(shell find $(SRC_DIR) -name '*.c')
INTERRUPT_C_FILES = $(SRC_DIR)/InterruptHandlers.c
ASM_FILES         = $(shell find $(SRC_DIR) -name '*.s')

OBJ_C_FILES   = $(patsubst $(SRC_DIR)/%.c,$(TARGET_DIR)/%.o,$(C_FILES))
OBJ_ASM_FILES = $(patsubst $(SRC_DIR)/%.s,$(TARGET_DIR)/%.o,$(ASM_FILES))

OBJ_FILES = $(OBJ_C_FILES) $(OBJ_ASM_FILES)

.PHONY: all clean

all: $(TARGET)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

# Checks if the compiled file contains interrupt routines and if so adds a special flag
$(TARGET_DIR)/%.o: $(SRC_DIR)/%.c | $(TARGET_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(MODE) $(CCFLAGS) $(if $(filter $(INTERRUPT_C_FILES),$^),$(INTERRUPT_CCFLAGS),) -c $< -o $@

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.s | $(TARGET_DIR)
	@mkdir -p $(dir $@)
	$(AS) $< -o $@

$(TARGET): $(OBJ_FILES)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(TARGET_DIR)

