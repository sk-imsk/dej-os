#!/usr/bin/env bash
qemu-system-x86_64 -drive format=raw,file=build/dej-os.img -no-reboot -serial mon:stdio -smp 4 -cpu max -M q35 -audio driver=pa,model=hda
