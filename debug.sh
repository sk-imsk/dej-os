#!/usr/bin/env bash
echo run gdb other teminal
qemu-system-x86_64 -drive format=raw,file=build/dej-os.img  -no-reboot  -no-shutdown  -S  -gdb tcp::1234
