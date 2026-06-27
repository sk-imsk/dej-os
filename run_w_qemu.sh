#!/usr/bin/env bash
qemu-system-i386 -drive format=raw,file=build/main_floppy.img,if=floppy
