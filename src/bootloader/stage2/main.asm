org 0x20000
bits 16


%define ENDL 0x0D, 0x0A



entry:
    mov ax, cs
    mov ds, ax



    mov ss, ax
    mov sp, 0xFFFE





enablea20:
    inc si

    mov ax, 0x2401
    int 0x15
    jnc aftera20

    cmp si, 3
    jz a20_error
    jmp enablea20

aftera20:


    sti
    mov si, a20_success_s
    call puts



    mov [ebr_drive_number], dl
    mov ax, 1
    mov cl, 1
    mov bx, 0x7E00    ; free space after boot loader
    call disk_read

    mov si, dih
    call puts




    ; not done yet
    cli
    hlt


;
; Disk routines
;

;
; lba to chs
; ax - lba address
; returns
; cx [bits 0-5 ]: sector number
; cx [bits 6-15]: cylinder
; dh: head
;

lba_to_chs:


	push ax
	push dx
	push ds
	xor ax, ax
	mov fs, ax



	xor dx, dx				; dx = 0
	div word fs:[0x7C18]	; ax = lba % sectors per track
						; dx = lba % sectors per track
	inc dx					; dx = (lba % sectors per track + 1) = sector
	mov cx, dx				; cx = sector


	xor dx, dx				; dx = 0
	div word fs:[0x7C1A]			; ax = (lba / sectors per track) / heads = cylinder
						; dx = (lba / sectors per track) % heads = head



	mov dh, dl				; dl = head
	mov ch, al 				; ch = cylinder

	shl ah, 6
	or cl, ah				; put upper 2 bits of cylinder in cl


	pop ds
	pop dx
	pop ax


	ret

;Reads sector from disk
;   Parameters
;   - ax lba address
;   - cl number of sectors to read max 128
;   - dl drive number
;   - es:bx memory address to store data
;
;
disk_read:



	push cx
	call lba_to_chs ;compute chs
	pop ax          ; al = num of sectors to read




	mov ah, 02h
	mov di, 3       ; retry number


.retry:
    pusha
    stc
    mov dl, [ebr_drive_number]
	int 13h
	jnc .done


	;read failed
	popa
	call disk_reset
	dec di
	test di, di
	jnz .retry

.fail:
    jmp floppy_error

.done:
    popa

    ret


;
; disk reset
; dl is drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret

;
; put string string pointer in si
;
puts:
	; save regsiters lol
	push ax
	push si


    .loop:
	lodsb ; loads character in al
	or al, al; verify there is a next character
	jz .done

	mov ah, 0x0e     ; call big boi interupt
	int 0x10


	jmp .loop ; keep going beacuse there is another character to print (get back to work)


    .done:
	pop si
     	pop ax
	ret


floppy_error:
    cli
    mov si, floppy_error_s
    call puts

    call halt

a20_error:
    ; error
    cli
    mov si, a20_error_s
    call puts

    call halt


; must use cli before
halt:
    hlt
    jmp halt




ebr_drive_number: db 0
a20_error_s: db "Failed to init a20", ENDL, 0
a20_success_s: db "a20 enabled", ENDL, 0
floppy_error_s: db "failed to read kernel from disk", ENDL, 0
dih: db "dih", ENDL, 0
msg1: db "1", ENDL, 0
msg2: db "2", ENDL, 0
msg3: db "3", ENDL, 0
msg4: db "4", ENDL, 0
