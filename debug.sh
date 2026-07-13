#!/usr/bin/env bash
echo run gdb other teminal
qemu-system-i386 -drive format=raw,file=build/main_floppy.img,if=floppy -S -s
