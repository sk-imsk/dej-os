#!/usr/bin/env bash
echo run gdb other teminal
qemu-system-x86_64 -drive format=raw,file=build/dej-os.img  -no-reboot  -no-shutdown -M q35  -S  -gdb tcp::1234 -d int -smp 4 -audio driver=pa,model=hda -cpu max
