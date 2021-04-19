[bits 64]
global syscall_wrapper

syscall_wrapper:
    mov rax, rcx
    syscall
    ret