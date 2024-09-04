TARGET_DIR = out
SRC_DIR = src
INC_DIR = inc
TARGET = $(TARGET_DIR)/Kernel.elf

CXX = x86_64-elf-g++
LD  = x86_64-elf-ld

CXXFLAGS = \
	-I $(INC_DIR) \
	-std=c++23 \
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

LDFLAGS = \
	-T linker.ld \
	-static \
	-z max-page-size=0x1000 \
	-nostdlib

SRC_FILES = $(shell find $(SRC_DIR) -name '*.cpp')

OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(TARGET_DIR)/%.o,$(SRC_FILES))

.PHONY: all clean

all: $(TARGET)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.cpp | $(TARGET_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(TARGET_DIR)

