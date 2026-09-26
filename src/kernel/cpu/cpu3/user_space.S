

.global jmp2user
jmp2user:
    # rdi = user RIP
    # rsi = user RSP

    mov user_cr3(%rip), %rax
    mov %rax, %cr3

    pushq $0x43          # USER SS
    pushq %rsi           # USER RSP

    pushfq
    popq %rax
    orq $0x200, %rax
    pushq %rax

    pushq $0x3B          # USER CS
    pushq %rdi           # USER RIP

    iretq
