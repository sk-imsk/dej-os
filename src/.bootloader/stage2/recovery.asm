bits 16

%define ENDL 0x0D, 0x0A


;
; recovery screen
; bh = error code
; bl = depends of error, check inside function
; never returns
;
recovery:

    cld
    push ax
    mov ax, cs
    mov ds, ax
    mov es, ax
    pop ax


    ; setup

    mov [error_code], bh
    mov [error_d], bl



    mov ax, 0x0600
    mov bh, 0x2F
    mov cx, 0x0000
    mov dx, 0x184F
    int 10h

    mov ah, 0x02
    mov bh, 0
    mov dh, 2
    mov dl, 30
    int 10h

    mov si, title
    call puts

    mov ah, 0x02
    mov dh, 10
    mov dl, 0
    int 10h

    mov ah, 0x02

    int 10h

    ; error codes

    cmp byte [error_code], 1
    je mem_init

    cmp byte [error_code], 2
    je disk

    cmp byte [error_code], 3
    je missing_file

    jmp unknown

unknown:            ; (0)
    mov si, unknown_s
    call puts
    jmp halt




missing_file:   ; 3


    mov si, missing_file_s
    call puts


    cmp byte [error_d], 1
    je .kernel_bin

    jmp halt

.kernel_bin:
    mov si, missing_kernel_s
    call puts
    jmp halt

disk:               ; 2
    mov si, disk_s
    call puts
    jmp halt



mem_init:           ; 1
    mov si, mem_init_s
    call puts

    jmp halt





halt:
    hlt
    jmp halt




print_digit:
    add al, '0'
    mov ah, 0x0E
    mov bh, 0
    int 10h
    ret


print_hex:
    push ax

    mov ah, 0Eh

    mov bl, al
    shr al, 4
    call .digit

    mov al, bl
    and al, 0Fh

.digit:
    cmp al, 9
    jbe .num
    add al, 7
.num:
    add al, '0'
    int 10h

    pop ax
    ret

title: db "Recovery", ENDL, 0
unknown_s: db "Unknown error", ENDL, 0
mem_init_s: db "Failed to initilise memory", ENDL, 0
disk_s: db "The disk could not be accessed", ENDL, 0
missing_file_s: db "A required file was not able to be opened", ENDL, 0
missing_kernel_s: db "Missing file is dkrnl.sys", ENDL, 0
error_code: db 0
error_d: db 0
