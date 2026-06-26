@echo off
del /q kasm.o kc.o kernel.bin 2>nul
wsl bash -c "which nasm qemu-system-x86_64 >/dev/null 2>&1 || (sudo apt update && sudo apt install -y nasm qemu-system-x86)"
if errorlevel 1 goto err
wsl bash ./build.sh
if errorlevel 1 goto err
wsl qemu-system-x86_64 -display curses -kernel kernel.bin
if errorlevel 1 goto err
exit /b 0
:err
pause
exit /b 1

