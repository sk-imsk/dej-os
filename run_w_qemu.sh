#!/usr/bin/env bash
qemu-system-x86_64 -drive format=raw,file=build/dej-os.img -no-reboot -serial mon:stdio -smp 2 -cpu max
