bits 16

section _TEXT class=CODE

global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:
    push bp         ; save old call
    mov bp, sp      ; init new call

    ; save bx
    push bx

    ; bp is old call frame
    ; [bp + 2] return address
    ; [bp + 4] first arg   (character)     ; bytes are convert to word beacuse fuck you
    ; [bp + 6] second arg (page)
    ; all bytes are converted to word
    mov ah, 0Eh
    mov al, [bp + 4]; first argument
    mov bh, [bp + 6] ; omg second argument

    int 10h
    ;revive bx
    pop bx

    ;restore old call frame
    mov sp, bp
    pop bp
    ret
