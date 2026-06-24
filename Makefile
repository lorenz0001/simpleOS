# Directories
SRC_DIR = src
BUILD_DIR = build

# Files
BOOT_SRC = $(SRC_DIR)/bootloader.asm
KERNEL_SRC = $(SRC_DIR)/kernel.asm
BOOT_BIN = $(BUILD_DIR)/bootloader.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMG = $(BUILD_DIR)/os.img

# Assembler
ASM = nasm
ASM_FLAGS = -f bin -i $(SRC_DIR)/

.PHONY: all clean run

all: $(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BOOT_BIN): $(BOOT_SRC) $(SRC_DIR)/print.asm $(SRC_DIR)/disk.asm | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS) $< -o $@

$(KERNEL_BIN): $(KERNEL_SRC) $(SRC_DIR)/print.asm $(SRC_DIR)/keyboard.asm $(SRC_DIR)/string.asm | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS) $< -o $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	# Create a blank floppy disk image (1.44 MB)
	dd if=/dev/zero of=$(OS_IMG) bs=512 count=2880
	# Write bootloader to sector 0 (512 bytes)
	dd if=$(BOOT_BIN) of=$(OS_IMG) conv=notrunc
	# Write kernel starting from sector 1 (the second sector)
	dd if=$(KERNEL_BIN) of=$(OS_IMG) seek=1 conv=notrunc

run: $(OS_IMG)
	@echo "Starting QEMU (GUI mode)..."
	qemu-system-x86_64 -fda $(OS_IMG)

run-curses: $(OS_IMG)
	@echo "Starting QEMU in curses mode..."
	@echo "Press 'Esc' then '2' to switch to monitor console, or 'Esc' then '1' for main OS screen."
	qemu-system-x86_64 -fda $(OS_IMG) -display curses

clean:
	rm -rf $(BUILD_DIR)
