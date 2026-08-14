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
bdb_sectors_per_cluster: 	db 1
bdb_reserved_sectors: 		dw 1
bdb_fat_count: 			    db 2
bdb_dir_entries_count:		dw 0E0H
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




;
;
; Prints a string to the screen
;
; - ds:si points to string
;



main:

    cli
	; setup
	mov ax, 0
	mov ds, ax
	mov es, ax

	; setup stack
	mov ss, ax
	mov sp, 0x7C00
	sti


	; some bios might start at 07c0:0000 instead of 0000:7C00
	; make sure we are in the right spot
	push es
	push word .after
	retf


.after:
	; read something lol
	; bios should set dl to drive number
	mov [ebr_drive_number], dl

	; show loading message
	mov si, msg_loading
	call puts

	; read drive Parameters (sectors per track and heads),
	; instead of relying on data in formatted disk
	; this section can be hard coded
	push es
	mov ah, 08h
	int 13h
	jc floppy_error
	pop es

	and cl, 0x3F            ;remove top 2 bits
	xor ch, ch
	mov [bdb_sectors_per_track], cx ;sector count

	inc dh
	mov [bdb_heads], dh         ; head count

	; find lba = reserved + fats * sectors per fat
	mov ax, [bdb_sectors_per_fat]
	mov bl, [bdb_fat_count]
	xor bh, bh
	mul bx                                          ; ax = (fats * sectors per fat)
	add ax, [bdb_reserved_sectors]                  ; ax = lba of root dir
	push ax

	; get size of root directory = (32 * number of entries) / bytes_per_sector
	mov ax, [bdb_dir_entries_count]
	shl ax, 5                                       ; ax *= 32
	xor dx, dx                                      ;dx = 0
	div word [bdb_bytes_per_sector]                 ;num of sectors we need to read

	test dx, dx                                     ; if dx != 0 add 1
	jz .root_dir_after
	inc ax                                          ; means we only have a sector part filled with entries


.root_dir_after:
    ; read root dir
    mov cl, al                                      ; cl = number of sectors to read = size of root dir
    pop ax                                          ; ax = lba of root dir
    mov dl, [ebr_drive_number]                      ; dl = drive number
    mov bx, buffer                                  ; es: bx = buffer
    call disk_read


    ; search for stage2.bin
    xor bx, bx
    mov di, buffer

.search_stage2:
    mov si, file_stage2_bin
    mov cx, 11                                  ; comp 11 characters
    push di
    repe cmpsb
    pop di
    je .found_stage2


    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_stage2

    ; no stage2 bootloader found
    jmp stage2_not_found_error

.found_stage2:

    ; di should have the address to entry
    mov ax,  [di + 26]                   ; first logical cluster
    mov [stage2_cluster], ax

    ; load fat from disk into memory
     mov ax, [bdb_reserved_sectors]
     mov bx, buffer
     mov cl, [bdb_sectors_per_fat]
     mov dl, [ebr_drive_number]
     call disk_read

     ; read stage2 and process fat chain
     mov bx, STAGE2_LOAD_SEGMENT
     mov es, bx
     mov bx, STAGE2_LOAD_OFFSET

.load_stage2_loop:
    ; read next cluster
    mov ax, [stage2_cluster]
    ; idk its hard coded
    add ax, 31                             ; first cluster =  (cluster number  -2 ) * sectors_per_cluster + stage2_cluster
                                           ; start sector = reserved + fats +  root dir size = 1 + 18 + 134 = 33

    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, [bdb_bytes_per_sector]

    ; find next cluster
    mov ax, [stage2_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even

.odd:
	shr ax, 4
	jmp .next_cluster_after

.even:
    and ax, 0x0FFF

.next_cluster_after:
    cmp ax, 0x0FF8
    jae .read_finish

    mov [stage2_cluster], ax
    jmp .load_stage2_loop


.read_finish:
    ; jump to  stage 2
    mov dl, [ebr_drive_number]

    ;set segment registers
    mov ax, STAGE2_LOAD_SEGMENT
    mov ds, ax
    mov es, ax

    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

    jmp wait_key_and_reboot ; should not happen or something broke


	cli
	hlt

floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

stage2_not_found_error:
     mov si, msg_stage2_not_found
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



msg_loading: db 'Loading', ENDL, 0
msg_read_failed: db 'Cant read from the disk', ENDL, 0
msg_stage2_not_found: db 'Stage 2 bootloader not found', ENDL, 0
file_stage2_bin: db 'STAGE2  BIN'
stage2_cluster: dw 0


STAGE2_LOAD_SEGMENT         equ 0x2000
STAGE2_LOAD_OFFSET          equ  0

times 510-($-$$) db 0
dw 0AA55h

buffer:
