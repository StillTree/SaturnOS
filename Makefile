TARGET_DIR = out
SN_TARGET  = $(TARGET_DIR)/bootx64.efi
SK_TARGET  = $(TARGET_DIR)/Kernel.elf

SN_DIR      = Bootloader
SK_DIR      = Kernel
ESP_DIR     = esp
SN_DATA_DIR = Supernova

COMPONENTS = $(SN_DIR) $(SK_DIR)

.PHONY: all $(COMPONENTS) esp clean

all: $(COMPONENTS) esp

$(COMPONENTS):
	$(MAKE) -C $@

esp:
	mkdir -p $(ESP_DIR)/efi/boot/
	mkdir -p $(ESP_DIR)/$(SN_DATA_DIR)
	cp $(SN_DIR)/$(SN_TARGET) $(ESP_DIR)/efi/boot/
	cp $(SK_DIR)/$(SK_TARGET) $(ESP_DIR)/$(SN_DATA_DIR)/

clean:
	@for dir in $(COMPONENTS); do \
		$(MAKE) -C $$dir clean;   \
	done
	rm -rf $(ESP_DIR)

