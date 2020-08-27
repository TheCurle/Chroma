
mmio = 0xfffffffff8000000;
fb   = 0xfffffffffc000000;

PHDRS
{
	boot PT_LOAD;
}

SECTIONS
{
	. = 0xFFFFFFFFFFE00000;
	LoadAddr = .;
	bootldr = .; . += 4096;
	environment = .; . += 4096;


	.text : {
		KEEP(*(.text.boot)) *(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
	} :boot
	.bss (NOLOAD) : {
		. = ALIGN(16);
		*(.bss .bss.*)
		*(COMMON)
	} :boot

	.eh_frame_hdr : {
		*(.eh_frame_hdr)
		*(.eh_frame)
		*(.gcc_except_table)
	} :boot

	/DISCARD/ : {*(.comment) }

	end = .;
}
