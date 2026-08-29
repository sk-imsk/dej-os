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

extern divide_by_0_handler
global int_divide_by_0
int_divide_by_0:
    PUSH_ALL


    call divide_by_0_handler


    POP_ALL
    iretq

extern nmi_handler
global int_nmi
int_nmi:
    PUSH_ALL

    mov rdi, rsp

    call nmi_handler


    POP_ALL
    iretq

extern general_protection_fault_handler
global int_general_protection_fault
int_general_protection_fault:
    PUSH_ALL

    call general_protection_fault_handler


    POP_ALL
    iretq



extern page_fault_handler
global int_page_fault
int_page_fault:
    PUSH_ALL


    call page_fault_handler


    POP_ALL
    iretq
