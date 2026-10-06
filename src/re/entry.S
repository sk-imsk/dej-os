.code64
.global _start
.section .text


_start:
    pushq %rax
    pushq %rbx


    subq $8, %rsp

    call main

    cli
_hlt:
    hlt
    jmp _hlt
