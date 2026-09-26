.global load_tss
.type load_tss, @function

load_tss:
    mov $0x48, %ax
    ltr %ax
    ret
