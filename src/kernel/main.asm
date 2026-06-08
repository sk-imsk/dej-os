org 0x0000
bits 16

%define ENDL 0x0D, 0x0A

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

	; setup
	mov ax, 0x2000
	mov ds, ax
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
