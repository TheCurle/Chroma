[BITS 32] ;... somehow.

[GLOBAL start]
start:
	mov esp, _sys_stack ; Stack pointer!
	jmp stublet         ; This has a purpse. I don't know what it is, but there is one.

ALIGN 4 ; 4KB alignment, required by GRUB.
mboot:  ; This is all magic, and all of it required for GRUB to see our stuff.
	MULTIBOOT_ALIGN     equ 1<<0
	MULTIBOOT_MEM       equ 1<<1
	MULTIBOOT_AOUT      equ 1<<16
	MULTIBOOT_MAGIC     equ 0x1BADB002
	MULTIBOOT_FLAGS     equ MULTIBOOT_ALIGN | MULTIBOOT_MEM | MULTIBOOT_AOUT
	MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
	EXTERN code, bss, end
	
	dd MULTIBOOT_MAGIC
	dd MULTIBOOT_FLAGS
	dd MULTIBOOT_CHECKSUM

	dd mboot
	dd code
	dd bss
	dd end
	dd start

stublet: ; Where the kernel stuff goes.
	
	;=====================================
	;===Priority: 32 bit Protected Mode===
	;=====================================
	
	cli                             ; Interrupts be gone!
	
	xor cx, cx                      ; CX - GP, useful here.
kbempty:
	in al, 64h                      ; Read keyboard status
	test al, 02h			; Check if the buffer is full
	loopnz kbempty			; Wait until it is
	mov al, 0d1h			; Prepare a message
	out 64h, al			; And then send it to the keyboard (controller)
kbempty2:
	in al, 64h			; Read the status again
	test al, 02h			; Check if it's processed our message
	loopnz kbempty2			; And wait until it has
	mov al, 0dfh			; Prepare a different message, telling it to enable A20
	out 60h, al			; Send the message
	mov cx, 14h			; Restore the value of CX
wait_kb:				; Insinuate a 25uS delay to allow the processor to catch up
	out 0edh, ax			
	loop wait_kb	

	mov eax, cr0			; Read the control register
	or al, 1			; Set bit 1: protected mode
	mov cr0, eax			; Set the control register back
	jmp $+2				; Clear the queue
	nop				; (jump straight to kernel)
	nop

;==================================
;===32-bit Protected Mode active===
;==================================	
	; Call the kernel
	
	EXTERN kernel_main
	call kernel_main
	jmp $

[GLOBAL load_gdt]
[EXTERN gp]
load_gdt:
    lgdt [gp]        ; Load the new GDT pointer

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush
flush:
    ret

[GLOBAL idt_load]
[EXTERN idtp]
idt_load:
	lidt [idtp]
	ret


SECTION .bss
	resb 8192
_sys_stack:
