bits 16

section _ENTRY class=CODE

extern _cstart
global entry


entry:
    cli
    mov ax, ds
    mov ss, ax
    mov sp, 0
    mov bp, sp
    sti

    ; expect boot drive in dl send it as a arg to main

    xor dh, dh
    push dx
    call _cstart

    cli
    hlt
