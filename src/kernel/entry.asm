bits 64

global _start
extern kentry

section .bss
align 16

stack_bottom:
    resb 16384
stack_top:

section .text

_start:
    mov rsp, stack_top
    xor rbp, rbp

    call kentry

.hang:
    cli
    hlt
    jmp .hang
