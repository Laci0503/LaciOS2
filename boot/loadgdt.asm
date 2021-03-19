;[bits 64]
global load_gdt    ;From https://blog.llandsmeer.com/tech/2019/07/21/uefi-x64-userland.html
load_gdt:
    ;jmp $
    lgdt [rdi]      ; load GDT, rdi (1st argument) contains the gdt_ptr
    ;mov ax, 0x40    ; TSS segment is 0x40
    ;ltr ax          ; load TSS
    mov ax, 0x10    ; kernel data segment is 0x10
    mov ds, ax      ; load kernel data segment in data segment registers
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop qword rdi        ; pop the return address
    mov rax, 0x08   ; kernel code segment is 0x08
    push qword rax       ; push the kernel code segment
    push qword rdi       ; push the return address again
    ;jmp $
    db 0x48,0xcb           ; do a far return, like a normal return but
                    ; pop an extra argument of the stack
                    ; and load it into CS
global load_pml4
load_pml4:
    mov rax, 0x000ffffffffff000 ;bitmask
    and rdi, rax
    mov cr3, rdi
    ret

global jmp_to_kernel
jmp_to_kernel:
    ;jmp $
    mov rsp, rdi
    mov rbp, rsp
    mov rax, rsi
    mov rdi, rdx
    call rax