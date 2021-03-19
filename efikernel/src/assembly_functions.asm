;[bits 64]
.text:

;HARDWARE_INT_OFFSET equ 0x20
;global sin
;sin:
    ;mov st7, st0
;    jmp $
%macro interruptMacro 1
    global interrupts_int%1
    interrupts_int%1:
        ;jmp $
        ;iretq
        ;push rax
        ;push rdx
        ;mov ax, 0x3f8
        ;mov dh, 0
        ;mov dl, 'I'
        ;out dx, ax
        ;mov dl, 'N'
        ;out ax, dl
        ;mov dl, 'T'
        ;out dx, ax
        ;pop rdx
        ;pop rax

        push rax
        mov rax, interruptNumber
        mov qword [rax], %1
        pop rax
        jmp interrupts.intHandler
%endmacro
;
interruptMacro 0
interruptMacro 1
interruptMacro 2
interruptMacro 3
interruptMacro 4
interruptMacro 5
interruptMacro 6
interruptMacro 7
interruptMacro 8
interruptMacro 9
interruptMacro 10
interruptMacro 11
interruptMacro 12
interruptMacro 13
interruptMacro 14
interruptMacro 15
interruptMacro 16
interruptMacro 17
interruptMacro 18
interruptMacro 19
interruptMacro 20
interruptMacro 21
interruptMacro 22
interruptMacro 23
interruptMacro 24
interruptMacro 25
interruptMacro 26
interruptMacro 27
interruptMacro 28
interruptMacro 29
interruptMacro 30
interruptMacro 31
interruptMacro 32
interruptMacro 33
interruptMacro 34
interruptMacro 35
interruptMacro 36
interruptMacro 37
interruptMacro 38
interruptMacro 39
interruptMacro 40
interruptMacro 41
interruptMacro 42
interruptMacro 43
interruptMacro 44
interruptMacro 45
interruptMacro 46
interruptMacro 47
interruptMacro 48
interruptMacro 49
interruptMacro 50
interruptMacro 51
interruptMacro 52
interruptMacro 53
interruptMacro 54
interruptMacro 55
interruptMacro 56
interruptMacro 57
interruptMacro 58
interruptMacro 59
interruptMacro 60
interruptMacro 61
interruptMacro 62
interruptMacro 63
interruptMacro 64
interruptMacro 65
interruptMacro 66
interruptMacro 67
interruptMacro 68
interruptMacro 69
interruptMacro 70
interruptMacro 71
interruptMacro 72
interruptMacro 73
interruptMacro 74
interruptMacro 75
interruptMacro 76
interruptMacro 77
interruptMacro 78
interruptMacro 79
interruptMacro 80
interruptMacro 81
interruptMacro 82
interruptMacro 83
interruptMacro 84
interruptMacro 85
interruptMacro 86
interruptMacro 87
interruptMacro 88
interruptMacro 89
interruptMacro 90
interruptMacro 91
interruptMacro 92
interruptMacro 93
interruptMacro 94
interruptMacro 95
interruptMacro 96
interruptMacro 97
interruptMacro 98
interruptMacro 99
interruptMacro 100
interruptMacro 101
interruptMacro 102
interruptMacro 103
interruptMacro 104
interruptMacro 105
interruptMacro 106
interruptMacro 107
interruptMacro 108
interruptMacro 109
interruptMacro 110
interruptMacro 111
interruptMacro 112
interruptMacro 113
interruptMacro 114
interruptMacro 115
interruptMacro 116
interruptMacro 117
interruptMacro 118
interruptMacro 119
interruptMacro 120
interruptMacro 121
interruptMacro 122
interruptMacro 123
interruptMacro 124
interruptMacro 125
interruptMacro 126
interruptMacro 127
interruptMacro 128
interruptMacro 129
interruptMacro 130
interruptMacro 131
interruptMacro 132
interruptMacro 133
interruptMacro 134
interruptMacro 135
interruptMacro 136
interruptMacro 137
interruptMacro 138
interruptMacro 139
interruptMacro 140
interruptMacro 141
interruptMacro 142
interruptMacro 143
interruptMacro 144
interruptMacro 145
interruptMacro 146
interruptMacro 147
interruptMacro 148
interruptMacro 149
interruptMacro 150
interruptMacro 151
interruptMacro 152
interruptMacro 153
interruptMacro 154
interruptMacro 155
interruptMacro 156
interruptMacro 157
interruptMacro 158
interruptMacro 159
interruptMacro 160
interruptMacro 161
interruptMacro 162
interruptMacro 163
interruptMacro 164
interruptMacro 165
interruptMacro 166
interruptMacro 167
interruptMacro 168
interruptMacro 169
interruptMacro 170
interruptMacro 171
interruptMacro 172
interruptMacro 173
interruptMacro 174
interruptMacro 175
interruptMacro 176
interruptMacro 177
interruptMacro 178
interruptMacro 179
interruptMacro 180
interruptMacro 181
interruptMacro 182
interruptMacro 183
interruptMacro 184
interruptMacro 185
interruptMacro 186
interruptMacro 187
interruptMacro 188
interruptMacro 189
interruptMacro 190
interruptMacro 191
interruptMacro 192
interruptMacro 193
interruptMacro 194
interruptMacro 195
interruptMacro 196
interruptMacro 197
interruptMacro 198
interruptMacro 199
interruptMacro 200
interruptMacro 201
interruptMacro 202
interruptMacro 203
interruptMacro 204
interruptMacro 205
interruptMacro 206
interruptMacro 207
interruptMacro 208
interruptMacro 209
interruptMacro 210
interruptMacro 211
interruptMacro 212
interruptMacro 213
interruptMacro 214
interruptMacro 215
interruptMacro 216
interruptMacro 217
interruptMacro 218
interruptMacro 219
interruptMacro 220
interruptMacro 221
interruptMacro 222
interruptMacro 223
interruptMacro 224
interruptMacro 225
interruptMacro 226
interruptMacro 227
interruptMacro 228
interruptMacro 229
interruptMacro 230
interruptMacro 231
interruptMacro 232
interruptMacro 233
interruptMacro 234
interruptMacro 235
interruptMacro 236
interruptMacro 237
interruptMacro 238
interruptMacro 239
interruptMacro 240
interruptMacro 241
interruptMacro 242
interruptMacro 243
interruptMacro 244
interruptMacro 245
interruptMacro 246
interruptMacro 247
interruptMacro 248
interruptMacro 249
interruptMacro 250
interruptMacro 251
interruptMacro 252
interruptMacro 253
interruptMacro 254
interruptMacro 255

interruptNumber:
    dq 0
;esp_backup:
;    dq 0
;esp_backup2:
;    dq 0
;eax_backup:
;    dq 0
;global push_berfore_return
;push_berfore_return:
;    dq 0
;extern handleInterrupt
;global interrupts.intHandler
global current_state
current_state:
    cpu_rax:    dq 0
    cpu_rbx:    dq 0
    cpu_rcx:    dq 0
    cpu_rdx:    dq 0
    cpu_rsi:    dq 0
    cpu_rdi:    dq 0
    cpu_rbp:    dq 0
    cpu_r8:     dq 0
    cpu_r9:     dq 0
    cpu_r10:    dq 0
    cpu_r11:    dq 0
    cpu_r12:    dq 0
    cpu_r13:    dq 0
    cpu_r14:    dq 0
    cpu_r15:    dq 0
    times 80    db 0
    times 256   db 0
    cpu_error:  dq 0
    cpu_rip:    dq 0
    cpu_cs:     dq 0
    cpu_rflags: dq 0
    cpu_rsp:    dq 0
    cpu_ss:     dq 0

extern handleInterrupt
rbx_backup:
    dq 0
interrupts.intHandler:
    cli
    push rax
    ;mov ax, 0x10
    ;mov ss, ax
    mov rax, rbx_backup
    mov qword [rax], rbx
    mov rax, interruptNumber
    mov qword rbx, [rax]
    pop rax
;
    cmp qword rbx, 8
    je continue
    cmp qword rbx, 10
    je continue
    cmp qword rbx, 11
    je continue
    cmp qword rbx, 12
    je continue
    cmp qword rbx, 13
    je continue
    cmp qword rbx, 14
    je continue
    cmp qword rbx, 17
    je continue
    cmp qword rbx, 30
    je continue
    mov rbx, 0
    push rbx
    continue:

    push rbx
    mov rbx, rax
    mov rax, cpu_rax
    mov [rax], rbx
    pop rbx

    add rax, 8
    mov [rax], rbx
    add rax, 8
    mov [rax], rcx
    add rax, 8
    mov [rax], rdx
    add rax, 8
    mov [rax], rsi
    add rax, 8
    mov [rax], rdi
    add rax, 8
    mov [rax], rbp
    add rax, 8
    mov [rax], r8
    add rax, 8
    mov [rax], r9
    add rax, 8
    mov [rax], r10
    add rax, 8
    mov [rax], r11
    add rax, 8
    mov [rax], r12
    add rax, 8
    mov [rax], r13
    add rax, 8
    mov [rax], r14
    add rax, 8
    mov [rax], r15
    add rax, 344
    
    pop rbx
    mov [rax], rbx
    add rax, 8
    pop rbx
    mov [rax], rbx
    add rax, 8
    pop rbx
    mov [rax], rbx
    add rax, 8
    pop rbx
    mov [rax], rbx
    add rax, 8
    pop rbx
    mov [rax], rbx
    add rax, 8
    pop rbx
    mov [rax], rbx

    mov rax, interruptNumber
    mov rdi, [rax]
    mov rax, handleInterrupt
    call rax

    mov rax, cpu_ss
    mov rbx, [rax]
    push rbx
    sub rax, 8
    mov rbx, [rax]
    push rbx
    sub rax, 8
    mov rbx, [rax]
    push rbx
    sub rax, 8
    mov rbx, [rax]
    push rbx
    sub rax, 8
    mov rbx, [rax]
    push rbx
    
    mov rax, cpu_rbx
    mov rbx, [rax]
    add rax, 8
    mov rcx, [rax]
    add rax, 8
    mov rdx, [rax]
    add rax, 8
    mov rsi, [rax]
    add rax, 8
    mov rdi, [rax]
    add rax, 8
    mov r8,  [rax]
    add rax, 8
    mov r9,  [rax]
    add rax, 8
    mov r10, [rax]
    add rax, 8
    mov r11, [rax]
    add rax, 8
    mov r12, [rax]
    add rax, 8
    mov r13, [rax]
    add rax, 8
    mov r14, [rax]
    add rax, 8
    mov r15, [rax]

    push rbx
    mov rax, cpu_rax
    mov rbx, [rax]
    mov rax, rbx
    pop rbx

    push rax
    mov rax, cpu_cs
    mov rax, [rax]
    cmp rax, 0x8

    jne iretq_cpl3
    pop rax
    iretq

return:
    ;jmp $
    ;iretq
iretq_cpl3:
    pop rax
    push rcx
    mov rcx, rax ; save rax
    mov rax, cpl3_rax
    mov [rax], rcx
    pop rcx

    add rax, 8 ; save rcx
    mov [rax], rcx

    add rax, 8 ; save r11
    mov [rax], r11

    add rax, 8 ; pop and save rip
    pop rcx
    mov [rax], rcx

    add rsp, 8 ; dispose cs

    pop r11 ; rflags into r11 for sysret; save it for cpl3 code
    add rax, 8
    mov [rax], r11
    
    pop rax ; rsp
    mov rsp, rax ; rsp

    mov rcx, cpl3_returner ; go to cpl3_returner
    ;jmp $

    o64 sysret

global while1
while1:
    jmp $

global enable_sce
enable_sce:
    mov rcx, 0xc0000080
    rdmsr
    or eax, 1
    wrmsr
    mov rcx, 0xc0000081
    rdmsr
    mov edx, 0x00180008
    wrmsr
    ret

global enter_userspace
enter_userspace:
    mov rcx, rdi ; rip
    mov rsp, rsi ; rsp
    mov r11, rdx ; rflags
    ;jmp $
    
    o64 sysret

global load_pml4
load_pml4:
    mov rax, 0x000ffffffffff000 ;bitmask
    and rdi, rax
    mov cr3, rdi
    ret

global load_gdt
load_gdt:
    lgdt [rdi]      ; load GDT, rdi (1st argument) contains the gdt_ptr
    mov ax, 0x40    ; TSS segment is 0x40
    ltr ax          ; load TSS
    ret






;cpl3
align(4096)
global cpl3_kernel_part
global cpl3_reg_save
global cpl3_returner
cpl3_kernel_part:
cpl3_reg_save:
    cpl3_rax: dq 0
    cpl3_rcx: dq 0
    cpl3_r11: dq 0
    cpl3_rip: dq 0
    cpl3_rflags: dq 0
cpl3_returner:
    mov rax, cpl3_r11 ; restore r11
    mov r11, [rax]

    mov rax, cpl3_rcx ; restore rcx
    mov rcx, [rax]

    mov rax, cpl3_rip ; push rip on the stack for the return
    mov rax, [rax]
    push rax

    mov rax, cpl3_rflags ; push rflags onto the stack
    mov rax, [rax]
    push rax

    mov rax, cpl3_rax ; restore rax
    mov rax, [rax]

    popf ; pop the flags from the stack
    ret