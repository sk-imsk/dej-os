.section .data
.align 8

.global gdt_start
.global gdt_end
.global gdt_descriptor
.global __tss_descriptor_ptr

gdt_start:
    # Index 0: Null descriptor (Selector 0x00)
    .quad 0x0000000000000000

    # Index 1: 16-bit code (Selector 0x08)
    .word 0xFFFF, 0x0000; .byte 0x00, 0x9A, 0x00, 0x00

    # Index 2: 16-bit data (Selector 0x10)
    .word 0xFFFF, 0x0000; .byte 0x00, 0x92, 0x00, 0x00

    # Index 3: 32-bit code (Selector 0x18)
    .word 0xFFFF, 0x0000; .byte 0x00, 0x9A, 0xCF, 0x00

    # Index 4: 32-bit data (Selector 0x20)
    .word 0xFFFF, 0x0000; .byte 0x00, 0x92, 0xCF, 0x00

    # Index 5: 64-bit Kernel Code (Selector 0x28)
    # Access: 0x9A (Present, Ring 0, Code, Readable)
    # Flags:  0x20 (Long Mode L=1, D/B=0, Limit=0)
    .word 0x0000, 0x0000; .byte 0x00, 0x9A, 0x20, 0x00

    # Index 6: 64-bit Kernel Data (Selector 0x30)
    # Access: 0x92 (Present, Ring 0, Data, Writable)
    # Flags:  0x00
    .word 0x0000, 0x0000; .byte 0x00, 0x92, 0x00, 0x00

    # Index 7: 64-bit User Code (Selector 0x38 | RPL 3 = 0x3B)
    # Access: 0xFA (Present, Ring 3, Code, Readable)
    # Flags:  0x20 (Long Mode L=1)
    .word 0x0000, 0x0000; .byte 0x00, 0xFA, 0x20, 0x00

    # Index 8: 64-bit User Data (Selector 0x40 | RPL 3 = 0x43)
    # Access: 0xF2 (Present, Ring 3, Data, Writable)
    # Flags:  0x00
    .word 0x0000, 0x0000; .byte 0x00, 0xF2, 0x00, 0x00

    # Index 9 & 10: 64-bit TSS Descriptor (Selector 0x48, 16 bytes total)
    # Reserved space to be populated dynamically by write_tss_descriptor() in C
__tss_descriptor_ptr:
    .quad 0x0000000000000000
    .quad 0x0000000000000000

gdt_end:

gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .quad gdt_start
.section .text
.global load_gdt
.type load_gdt, @function

load_gdt:
    lgdt gdt_descriptor(%rip)

    # Set standard data segments
    mov $0x30, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    # Clear FS and GS so they don't clobber the hidden MSR base addresses
    xor %ax, %ax
    mov %ax, %fs
    mov %ax, %gs

    # Far return to reload CS
    pop %rax
    pushq $0x28
    pushq %rax
    lretq
