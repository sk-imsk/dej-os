bits 64

global x86_outb
x86_outb:
    mov dx, di
    mov al, sil
    out dx, al
    ret

global x86_inb
x86_inb:
    mov dx, di
    xor eax, eax
    in al, dx
    ret


global x86_load_idt
x86_load_idt:
    lidt [rdi]
    ret
