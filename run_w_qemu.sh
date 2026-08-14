#!/usr/bin/env bash
qemu-system-i386 -drive format=raw,file=build/dej-os.img,if=floppy -serial stdio
