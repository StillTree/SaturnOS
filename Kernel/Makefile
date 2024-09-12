TARGET_DIR = out
SRC_DIR    = src
INC_DIR    = inc
TARGET     = $(TARGET_DIR)/Kernel.elf

CXX = x86_64-elf-g++
AS  = x86_64-elf-as
LD  = x86_64-elf-ld

# TEMPORARY -mgeneral-regs-only !!!
CXXFLAGS = \
	-g \
	-I $(INC_DIR) \
	-std=c++23 \
	-mno-red-zone \
	-mgeneral-regs-only \
	-mno-mmx \
	-mcmodel=kernel \
	-mno-sse \
	-mno-sse2 \
	-fno-exceptions \
	-ffreestanding \
	-nostdlib \
	-Wall \
	-Wextra \
	-Wpedantic

LDFLAGS = \
	-T linker.ld \
	-static \
	-z max-page-size=0x1000 \
	-nostdlib

CPP_FILES = $(shell find $(SRC_DIR) -name '*.cpp')
ASM_FILES = $(shell find $(SRC_DIR) -name '*.s')

OBJ_CPP_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(TARGET_DIR)/%.o,$(CPP_FILES))
OBJ_ASM_FILES = $(patsubst $(SRC_DIR)/%.s,$(TARGET_DIR)/%.o,$(ASM_FILES))

OBJ_FILES = $(OBJ_CPP_FILES) $(OBJ_ASM_FILES)

.PHONY: all clean

all: $(TARGET)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.cpp | $(TARGET_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.s | $(TARGET_DIR)
	@mkdir -p $(dir $@)
	$(AS) $< -o $@

$(TARGET): $(OBJ_FILES)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(TARGET_DIR)

