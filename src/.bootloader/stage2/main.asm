org 0x20000
bits 16


%define ENDL 0x0D, 0x0A


entry:
    mov ax, cs
    mov ds, ax
    mov es, ax



    mov ss, ax
    mov sp, 0xFFFE
    mov si, 0
    jmp enablea20

%include "recovery.asm"



enablea20:
    inc si

    mov ax, 0x2401
    int 15h
    jnc load_kernel

    cmp si, 3
    jz a20_error
    jmp enablea20

load_kernel:
    xor ax, ax
    mov fs, ax

    sti
    mov si, a20_success_s
    call puts


    ; Load kernel from disk or sum
    mov [ebr_drive_number], dl
    push es
    mov ah, 08h
    int 13h
    jc floppy_error
    pop es

    mov si, s1
    call puts

    and cl, 0x3F            ;remove top 2 bits
	xor ch, ch
	mov [bdb_sectors_per_track], cx ;sector count

	inc dh
	mov [bdb_heads], dh         ; head count

                                            ;find lba = reserved + fats * sectors per fat
	mov ax, fs:[0x7C16]            ; sectors per fat
	mov bl, fs:[0x7C10]                  ; fat count
	xor bh, bh
	mul bx                                          ; ax = (fats * sectors per fat)
	add ax, fs:[0x7C0E]                  ; ax = lba of root dir (reserved sectors)
	push ax

	mov ax, word fs:[0x7C11]                  ; dir entry count
	shl ax, 5                            ; ax *= 32
	xor dx, dx                           ;dx = 0
	div word fs:[0x7C0B]                 ;num of sectors we need to read (bytes per sector)


	test dx, dx                                     ; if dx != 0 add 1
	jz root_dir_after
	inc ax



root_dir_after:
    ; read root dir
    mov cl, al                                      ; cl = number of sectors to read = size of root dir
    pop ax                                          ; ax = lba of root dir
    mov dl, [ebr_drive_number]                      ; dl = drive number
    mov bx, buffer                                  ; es: bx = buffer
    call disk_read


    ; search for dkrnl for the kernel
    xor bx, bx
    mov di, buffer

.search_kernel:
    cld
    mov si, file_dkrnl
    mov cx, 11                                  ; comp 11 characters
    push di
    repe cmpsb
    pop di
    je .found_kernel

    mov si, s4
    call puts

    add di, 32
    inc bx
    cmp bx, fs:[0x7C11] ; dir entry count
    jl .search_kernel

    ; no stage2 bootloader found
    mov bh, 3
    mov bl, 1
    jmp recovery



.found_kernel:

    ; di should have the address to entry
    mov ax,  [di + 26]                   ; first logical cluster
    mov [kernel_cluster], ax

    mov si, s5
    call puts

    ; load fat from disk into memory
    mov ax, word fs:[0x7C0E]        ; also reserved sectors
    mov bx, buffer
    mov cl, byte fs:[0x7C16]        ; sector per fat
    mov dl, [ebr_drive_number]
    call disk_read

    ; read stage2 and process fat chain
    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, KERNEL_LOAD_OFFSET



.load_kernel_loop:
    ; read next cluster
    mov ax, [kernel_cluster]
    ; idk its hard coded
    add ax, 31                             ; first cluster =  (cluster number  -2 ) * sectors_per_cluster + stage2_cluster
                                           ; start sector = reserved + fats +  root dir size = 1 + 18 + 134 = 33
    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, fs:[0x7C0B]

    ; find next cluster
    mov ax, [kernel_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even

    mov si, s6
    call puts

.odd:
	shr ax, 4
	jmp .next_cluster_after

.even:
    and ax, 0x0FFF

.next_cluster_after:
    cmp ax, 0x0FF8
    jae .read_finish

    mov [kernel_cluster], ax
    jmp .load_kernel_loop

.read_finish:
        ; jump to  stage 2
    mov dl, [ebr_drive_number]

        ;set segment registers
    mov ax, KERNEL_LOAD_SEGMENT
    mov ds, ax
    mov es, ax

    ; jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET (wait until we get 32 bit mode up)







    ; not done yet
hlt:
    cli
    hlt
    jmp hlt


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

	pop ax


	ret

;Reads sector from disk
;   Parameters
;   - ax lba address
;   - cl number of sectors to read max 128
;   - dl drive number
;   - es:bx memory address to store data
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
	int 10h


	jmp .loop ; keep going beacuse there is another character to print (get back to work)


.done:
	pop si
	pop ax
	ret


floppy_error:
    cli
    mov al, ah
    call print_hex
    mov bh, 2
    mov bl, 1
    jmp recovery

a20_error:
    ; error
    cli


    mov bh, 1
    jmp recovery






bdb_sectors_per_track: dw 18
bdb_heads: dw 2
ebr_drive_number: db 0
a20_success_s: db "a20 enabled", ENDL, 0
loading_s: db "Loading ...."
file_dkrnl: db "DKRNL    SYS"
KERNEL_LOAD_OFFSET:         equ 0x1000
KERNEL_LOAD_SEGMENT:        equ 0
kernel_cluster: dw 0
s1: db "1", ENDL, 0
s2: db "2", ENDL, 0
s3: db "3", ENDL, 0
s4: db "4", ENDL, 0
s5: db "5", ENDL, 0
s6: db "6", ENDL, 0


buffer:
