.section .text
.align 11
.global exception_vectors

exception_vectors:

    // Current EL, SP0
    b exception_handler
    .space 124

    b exception_handler
    .space 124

    b exception_handler
    .space 124

    b exception_handler
    .space 124

    // Current EL, SPx
    b exception_handler
    .space 124

    b exception_handler
    .space 124

    b exception_handler
    .space 124

    b exception_handler
    .space 124

    // Lower EL, AArch64
    b exception_handler
    .space 124

    b exception_handler
    .space 124

    b exception_handler
    .space 124

    b exception_handler
    .space 124

    // Lower EL, AArch32
    b exception_handler
    .space 124

    b exception_handler
    .space 124

    b exception_handler
    .space 124

    b exception_handler
    .space 124
