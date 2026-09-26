bits 64

_entry:
    jmp main



main:
    int 0x80
    jmp main
