TARGET_DIR = Output
SRC_DIR    = Source
INC_DIR    = Include
TARGET     = $(TARGET_DIR)/libSaturn.a

CC = clang
AS = clang
AR = llvm-ar

MODE ?= -O0 -g -DLS_DEBUG

CCFLAGS = \
	-I $(INC_DIR) \
	-std=c23 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-ffreestanding \
	-nostdlib \
	-nostdlibinc \
	-Wall \
	-Wextra \
	-Wpedantic

ARFLAGS = \
	rcs

C_FILES   = $(shell find $(SRC_DIR) -name '*.c')
ASM_FILES = $(shell find $(SRC_DIR) -name '*.s')

OBJ_C_FILES   = $(patsubst $(SRC_DIR)/%.c,$(TARGET_DIR)/%.o,$(C_FILES))
OBJ_ASM_FILES = $(patsubst $(SRC_DIR)/%.s,$(TARGET_DIR)/%.o,$(ASM_FILES))

OBJ_FILES = $(OBJ_C_FILES) $(OBJ_ASM_FILES)

.PHONY: all clean

all: $(TARGET)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.c | $(TARGET_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(MODE) $(CCFLAGS) -c $< -o $@

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.s | $(TARGET_DIR)
	@mkdir -p $(dir $@)
	$(AS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -rf $(TARGET_DIR)
