# Directories
SRC_DIR = src
BUILD_DIR = build

# Files
BOOT_SRC = $(SRC_DIR)/boot/bootloader.asm
BOOT_BIN = $(BUILD_DIR)/bootloader.bin
OS_IMG = $(BUILD_DIR)/os.img

# Assembler and compiler tools
ASM = nasm
CC = gcc
LD = ld

# Compiler & Linker Flags
ASM_FLAGS_BIN = -f bin -i $(SRC_DIR)/boot/
ASM_FLAGS_ELF = -f elf32 -i $(SRC_DIR)/boot/
CC_FLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -nostdlib -c -I$(SRC_DIR)/cpu -I$(SRC_DIR)/drivers -I$(SRC_DIR)/libc -I$(SRC_DIR)/kernel
LD_FLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary

# Objects for kernel linking (order of assembly objects at start is important)
KERNEL_OBJS = $(BUILD_DIR)/kernel_asm.o \
              $(BUILD_DIR)/ports.o \
              $(BUILD_DIR)/string.o \
              $(BUILD_DIR)/mem.o \
              $(BUILD_DIR)/fs.o \
              $(BUILD_DIR)/screen.o \
              $(BUILD_DIR)/keyboard.o \
              $(BUILD_DIR)/kernel_c.o

.PHONY: all clean run run-curses

all: $(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BOOT_BIN): $(BOOT_SRC) $(SRC_DIR)/boot/print.asm $(SRC_DIR)/boot/disk.asm | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS_BIN) $< -o $@

$(BUILD_DIR)/kernel_asm.o: $(SRC_DIR)/kernel/kernel.asm $(SRC_DIR)/boot/print.asm $(SRC_DIR)/boot/keyboard.asm $(SRC_DIR)/boot/string.asm | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS_ELF) $< -o $@

$(BUILD_DIR)/ports.o: $(SRC_DIR)/cpu/ports.asm | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS_ELF) $< -o $@

$(BUILD_DIR)/string.o: $(SRC_DIR)/libc/string.c $(SRC_DIR)/libc/string.h | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/mem.o: $(SRC_DIR)/libc/mem.c $(SRC_DIR)/libc/mem.h | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/fs.o: $(SRC_DIR)/kernel/fs.c $(SRC_DIR)/kernel/fs.h $(SRC_DIR)/libc/mem.h $(SRC_DIR)/libc/string.h | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/screen.o: $(SRC_DIR)/drivers/screen.c $(SRC_DIR)/drivers/screen.h $(SRC_DIR)/cpu/ports.h | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: $(SRC_DIR)/drivers/keyboard.c $(SRC_DIR)/drivers/keyboard.h $(SRC_DIR)/cpu/ports.h $(SRC_DIR)/drivers/screen.h | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/kernel_c.o: $(SRC_DIR)/kernel/kernel.c $(SRC_DIR)/cpu/ports.h $(SRC_DIR)/drivers/screen.h $(SRC_DIR)/drivers/keyboard.h $(SRC_DIR)/libc/string.h $(SRC_DIR)/libc/mem.h $(SRC_DIR)/kernel/fs.h | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $< -o $@

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LD_FLAGS) $^ -o $@

$(OS_IMG): $(BOOT_BIN) $(BUILD_DIR)/kernel.bin
	# Create a blank floppy disk image (1.44 MB)
	dd if=/dev/zero of=$(OS_IMG) bs=512 count=2880
	# Write bootloader to sector 0 (512 bytes)
	dd if=$(BOOT_BIN) of=$(OS_IMG) conv=notrunc
	# Write kernel starting from sector 1 (the second sector)
	dd if=$(BUILD_DIR)/kernel.bin of=$(OS_IMG) seek=1 conv=notrunc

run: $(OS_IMG)
	@echo "Starting QEMU (GUI mode)..."
	qemu-system-x86_64 -fda $(OS_IMG)

run-curses: $(OS_IMG)
	@echo "Starting QEMU in curses mode..."
	@echo "Press 'Esc' then '2' to switch to monitor console, or 'Esc' then '1' for main OS screen."
	qemu-system-x86_64 -fda $(OS_IMG) -display curses

clean:
	rm -rf $(BUILD_DIR)
