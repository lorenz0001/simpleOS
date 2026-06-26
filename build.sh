#!/bin/bash
set -e
nasm -f elf32 kernel.asm -o kasm.o
gcc -fno-stack-protector -fno-pie -ffreestanding -m32 -c kernel.c -o kc.o
ld -m elf_i386 -T link.ld -o kernel.bin kasm.o kc.o

