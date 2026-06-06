org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A



;
; fat 12 stuff lol
;
jmp short start
nop


bdb_oem: 			        db 'MSWIN4.1'
bdb_bytes_per_sector: 		dw 512
bdb_sectors_per_sector: 	db 1
bdb_reserved_sectors: 		dw 1
bdb_fat_count: 			    db 2
bdb_dir_entrys_count:		dw 0E0H
bdb_total_sectors:		    dw 2880
bdb_media_descriptor_type:	db 0F0h
bdb_sectors_per_fat:		dw 9
bdb_sectors_per_track:		dw 18
bdb_heads:			        dw 2
bdb_hidden_sectors:		    dd 0
bdb_large_sector_count:		dd 0


; extended boot record

ebr_drive_number:		db 0
				        db 0 					; reserved
ebr_signature:			db 29h
ebr_volume_id:			db 12h, 34h, 56h, 76h 			; baller serial number (no one cares)
ebr_volume_label:		db 'DEJ OS    '			; 11 byets  (i love spaces)
ebr_system_id:			db 'FAT_12  '



start:
	jmp main


puts:
	; save regsiters lol
	push si
	push ax

.loop:
	lodsb ; loads character in al
	or al, al; verify there is a next character
	jz .done

	mov ah, 0x0e     ; call big boi interupt
	int 0x10


	jmp .loop ; keep going beacuse there is another character to print (get back to work)


.done:
	pop ax
 	pop si
	ret




;
;
; Prints a string to the screen
;
; - ds:si points to string
;



main:

	; setup
	mov ax, 0
	mov ds, ax
	mov es, ax

	; setup stack
	mov ss, ax
	mov sp, 0x7C00

	; read something lol
	; bios should set dl to drive number
	mov [ebr_drive_number], dl
	mov ax, 1
	mov cx, 1
	mov bx, 0x7E00
	call disk_read


	mov si, msg_hello
	call puts

	cli
	hlt

floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h
    jmp 0FFFFh:0
    hlt

.halt:
    cli     ; disable interupts (no escape)
    hlt


;
; disk routines
;

;
; convert lba to chs address
; Parameters
; - ax: lba address
; returns:
; cx: [bits 0-5] sector number
; cx: [bits 6-15] cylinder
; dh: head
;

lba_to_chs:


	push ax
	push dx


	xor dx, dx				; dx = 0
	div word [bdb_sectors_per_track]	; ax = lba % sectors per track
						; dx = lba % sectors per track
	inc dx					; dx = (lba % sectors per track + 1) = sector
	mov cx, dx				; cx = sector

	xor dx, dx				; dx = 0
	div word [bdb_heads]			; ax = (lba / sectors per track) / heads = cylinder
						; dx = (lba / sectors per track) % heads = head

	mov dh, dl				; dl = head
	mov ch, al 				; ch = cylinder

	shl ah, 6
	or cl, ah				; put upper 2 bits of cylinder in cl

	pop ax
	mov dl, al
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
;Resets disk controler
; parameters:
; dl : drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret



msg_hello: db 'Hello world!', ENDL, 0
msg_read_failed: db "Cant read from the disk", ENDL, 0

times 510-($-$$) db 0
dw 0AA55h
