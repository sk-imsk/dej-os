

.global jmp2user
jmp2user:
    mov $0x28, %ax # user code section
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs


    pushq $0x28
    pushq %rsi

    pushfq
    popq %rax
    orq $0x200, %rax
    pushq %rax

    pushq $0x1b
    pushq %rdi

    iretq
