org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A



;
; fat 12 stuff lol
;
jmp short start
nop


bdb_oem: 			db 'MSWIN4.1'
bdb_bytes_per_sector: 		dw 512
bdb_sectors_per_sector: 	db 1
bdb_reserved_sectors: 		dw 1
bdb_fat_count: 			db 2
bdb_dir_entrys_count:		dw 0E0H		
bdb_total_sectors:		dw 2880
bdb_media_descriptor_type:	db 0F0h	
bdb_sectors_per_fat:		dw 9
bdb_sectors_per_track:		dw 18
bdb_heads:			dw 2
bdb_hidden_sectors:		dd 0
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
	mov dx, ax
	mov es, ax

	; setup stack
	mov ss, ax
	mov sp, 0x7C00
	
	mov si, msg_hello
	call puts

	hlt	
	

.hlt:
	jmp .hlt

msg_hello: db 'Hello world!', ENDL, 0

times 510-($-$$) db 0
dw 0AA55h



