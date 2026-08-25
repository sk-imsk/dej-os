%macro PUSH_ALL 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro



%macro POP_ALL 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

extern err_divide_by_0
global int_divide_by_0
int_divide_by_0:
    PUSH_ALL

    call err_divide_by_0

    POP_ALL
    iretq
extern err_general_protection_fault
global int_general_protection_fault
int_general_protection_fault:
    PUSH_ALL

    sub rsp, 8
    call err_general_protection_fault
    add rsp, 8

    POP_ALL
    iretq
extern err_page_fault
global int_page_fault
int_page_fault:
    PUSH_ALL

    call err_page_fault

    POP_ALL
    iretq
