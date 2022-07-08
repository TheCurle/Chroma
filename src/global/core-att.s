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

.code16
.equ BASE, 0x1000

.global stack
.extern initcpu

.extern coreidt
.extern CoreGDT

# 16-bit startup.
# Initialize registers.
# Load GDT
# Set flags
# Immediately jump to protected mode.

.global startCore
startCore:
    cli
    mov $0x0, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    lgdtl gdt_protected

    mov %cr0, %eax
    or $0x1, %ax
    mov %eax, %cr0

    ljmpl $0x8, $startCore32

.code32

# Protected mode setup.
# Set page tables
# Set PAE
# Immediately jump to long mode.

.section .text
startCore32:
    mov $0x10, %bx
    mov %bx, %ds
    mov %bx, %es
    mov %bx, %ss

    mov $0xA000, %eax
    mov %eax, %cr3

    mov %cr4, %eax          # Enable PAE
    or $32, %eax            # 1 << 5
    or $128, %eax           # 1 << 7
    mov %eax, %cr4

    mov $0xC0000080, %ecx
    rdmsr
    or $256, %eax           # 1 << 8
    wrmsr

    mov %cr0, %eax
    or $2147483648, %eax    # 1 << 31
    mov %eax, %cr0

    lgdt gdt_long
    ljmp $0x8, $startCore64

# Long mode setup.
# Prepare registers.
# Set flags
# Load the final GDT and IDT
# Jump to the leave function.

.code64
startCore64:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    mov $0x0, %ax
    mov %ax, %ds
    mov %ax, %gs

    lgdt CoreGDT
    lidt coreidt

    mov $0x0, %rbp
    push $0
    popf

    mov (leave), %rax
    jmp leave

# Final setup.
# Set some flags in registers.
# Jump into C++ code.

leave:
    push %rbp

    mov %cr0, %rax
    btr $2, %eax
    bts $1, %eax
    mov %rax, %cr0

    mov %cr4, %rax
    bts $9, %eax
    bts $10, %eax
    mov %rax, %cr4

    call initcpu

.global endCore
endCore:
