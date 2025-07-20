TARGET_DIR  = Output
SRC_DIR     = Source
INC_DIR     = Include
SYSROOT_DIR = ../SystemRoot
LIB_DIR     = $(SYSROOT_DIR)/Library/Kernel
TARGET      = $(TARGET_DIR)/Kernel.elf
COMPILER_RT = $(LIB_DIR)/libCompilerRT.a

CC = clang
AS = clang
LD = ld.lld

MODE ?= -O0 -g -DSK_DEBUG

CCFLAGS = \
	--sysroot=$(SYSROOT_DIR) \
	-I $(INC_DIR) \
	-std=c23 \
	-mno-red-zone \
	-mno-mmx \
	-mcmodel=kernel \
	-mno-sse \
	-mno-sse2 \
	-ffreestanding \
	-nostdlib \
	-nostdlibinc \
	-Wall \
	-Wextra \
	-Wpedantic

INTERRUPT_CCFLAGS = \
	-mgeneral-regs-only

LDFLAGS = \
	--sysroot=$(SYSROOT_DIR) \
	-T linker.ld \
	-static \
	-z max-page-size=0x1000 \
	-nostdlib \
	$(COMPILER_RT)

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
	$(AS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(TARGET_DIR)
