.section .text
.global _start
.global _init
.global main
.global _fini
.type _start, @function
_start:
	call _init
	call main
	call _fini