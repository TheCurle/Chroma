.intel_syntax noprefix
#************************
#*** Team Kitty, 2021 ***
#***     Chroma       ***
#************************

# Initial startup routines for new cores.
# New cores start in 16 bit real mode.
# Because of this, this is the only necessary assembler file in the OS.

# First, bring them up to Protected and Long mode.
# Then enable all necessary auxiliary features.
# Pass off to the CPP code to handle the heavy work, we just want the core running.

.intel_syntax

.code16
.equ BASE, 0x1000

.global stack
.extern initcpu

.extern coreidt
.extern coregdt

.global startCore
    cli
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    o32 lgdt [gdt_protected - startCore + BASE]

    mov eax, cr0
    or al, 0x1
    mov cr0, eax

    jmp 0x8:(startCore32 - startCore + BASE)

.code32

.section .text
startCore32:
    mov bx, 0x10
    mov ds, bx
    mov es, bx
    mov ss, bx
    
    mov eax, dword [0xa000] ; Load bootloader tables into the new core
    mov cr3, eax

    mov eax, cr4            ; Enable PAE
    or eax, 1 << 5 
    or eax, 1 << 7
    mov cr4, eax
    
    mov ecx, 0xc0000080
    rdmsr
    or eax, 1 << 8          ; Enable LME
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    lgdt [gdt_long - startCore + BASE]
    jmp 8:(startCore64 - startCore + BASE)

.code64
startCore64:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov ax, 0x0 ; These will contain core information later on
    mov fs, ax
    mov gs, ax

    lgdt [coregdt]
    lidt [coreidt]

    mov rbp, 0
    push 0
    popf

    mov rax, qword leave
    jmp leave

leave:
    push rbp
    mov rax, cr0
    btr eax, 2
    bts eax, 1
    mov cr0, rax

    mov rax, cr4
    bts eax, 9
    bts eax, 10
    mov cr4, rax

    call initcpu


align 16
    gdt_long:
        dw gdt_long_end - gdt_long_start - 1 ; gdt length
        dq gdt_long_start - startCore + BASE ; address of the gdt

align 16
    gdt_long_start:
        dq 0
        dq 0x00AF98000000FFFF
        dq 0x00CF92000000FFFF
    gdt_long_end:

align 16
    gdt_protected:
        dw gdt_protected_end - gdt_protected_start - 1 ; gdt length
        dd gdt_protected_start - startCore + BASE      ; address of the gdt
    
align 16
    gdt_protected_start:
        dq 0
        dq 0x00CF9A000000FFFF
        dq 0x00CF92000000FFFF
    gdt_protected_end:

align 16
    idt_protected:
        dw 0
        dd 0
        dd 0
        align 16

global endCore
endCore: