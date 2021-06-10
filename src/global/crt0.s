.section .text
.global _start
.global _init
.global Main
.global Exit
.type _start, @function
_start:
	push %rbp
	movq %rsp, %rbp
	sub $0x10, %rsp

	xor %rax, %rax
	call _init

	xor %rbp, %rbp
	
	xor %rax, %rax
	call Main

	movq %rax, %rdi
	call Exit
	