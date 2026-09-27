bits 64

_entry:
    jmp main



main:
    int 0x80

done:
    pause
    pause
    pause
    pause
    jmp done
