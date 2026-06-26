nasm -f bin boot.asm
qemu-system-x86_64 -fda boot -s -S
