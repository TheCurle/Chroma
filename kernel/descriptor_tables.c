/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/

/* This file combines the implementation
   of IDT and GDT, Interrupt Descriptor
   Table and Global Descriptor Table
   respectively.
   
   These are in the same file, because
   otherwise the GDT file would be
   roughly 10 lines long.
   
   This file contains the code to set up
   and configure the PIC, to configure and
   load the IDT and GDT through the processor,
   and other functions that are uniquely
   related to the above tasks. */

#include <kernel.h>

 /*
  * The GDT gate setting function.
  * Defines where the important parts of memory are.
  *    
  * @param  num:    GDT position ID.
  * @param  base:   Start of the memory block this entry is for.
  * @param  limit:  End of the above memory block.
  * @param  access: The ring number associated with this entry.
  * @param  gran:   Granularity - how big the assigned chunks of memory are.
  */
void gdt_set_gate(int num, uint32_t base,
				uint32_t limit, uint8_t access,
				uint8_t gran) {
		
		/* The function is mostly shifts and masks, nothing too special. */
		/* The gdt[] array is defined in the descriptor_tables header. */
		gdt[num].low_base = (base & 0xFFFF);
		gdt[num].middle_base = (base >> 16) & 0xFF;
		gdt[num].high_base = (base >> 24) & 0xFF;
		
		gdt[num].low_limit = (limit & 0xFFFF);
		gdt[num].granular = ((limit >> 16) & 0x0F);
		
		gdt[num].granular = (gran & 0xF0);
		gdt[num].access = access;
}

/*
	The function that puts it all together. It sets up the pointer,
	fills it in, and tells the CPU to load the new GDT immediately.
	*/
void gdt_install() {
	/* gp = GDT Pointer, defined in descriptor_tables header. */
	gp.limit = (sizeof(struct gdt_item) * 3) - 1;
	gp.base = (uint32_t)&gdt;
	
	/* The CPU requires that the first gate always be null. */
	gdt_set_gate(0, 0, 0, 0, 0);
	
	/*  This is the Code Segment, it starts at 0 and ends at 4GB, meaning
		that it encompasses the entirety of available memory.

		This defines the code segment with exclusive execute permission, 
	   allowing the kernel to execute itself. */
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	
	/* The Data Segment, the same range as the Code Segment (0 - 4GB)
	   but with Read/Write permission, allowing the kernel to modify RAM. */
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	
	/* Lastly, call the external ASM function that loads the GDT into the
	   processor. */
	load_gdt();
}

/*
    Identically to the GDT set gate function, but for the IDT array.
  * @param  num:   IDT access ID
  * @param  base:  Start of memory block
  * @param  sel:   Selector. I have no idea what that means.
  * @param  flags: Permission levels and all that stuff.
  */
void idt_set_gate(unsigned char num,
				unsigned long base,
				unsigned short sel,
				unsigned char flags) {
	/* Again, it's all shifts and masks. Nothing special. */
	idt[num].base_low = base & 0xFFFF;
	idt[num].base_high = (base >> 16) & 0xFFFF;
	
	idt[num].selector = sel;
	idt[num].always0 = 0;
	
	/* Set the permission level to 3. All interrupts are from ring 3. */
	idt[num].flags = flags | 0x60;
}

/*
	Same as GDT_install: sets up the pointers and fills in the contents, then passes
	it on to the CPU to be loaded.
 */
void idt_install() {
	/* idtp is the IDT Pointer, defined in the descriptor_tables header. */
	idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
	idtp.base = (uint32_t) &idt;
	
	memset(&idt, 0, sizeof(struct idt_entry) * 256);
	
	/* Manually fill in the array - these functions are defined in
		interrupts.c */

	idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
	idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
	idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
	idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
	idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
	idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
	idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
	idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);

	idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
	idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
	idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
	idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
	idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
	idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
	idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
	idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);

	idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
	idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
	idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
	idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
	idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
	idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
	idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
	idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);

	idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
	idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
	idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
	idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
	idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
	idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
	idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
	idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
	
	idt_load();
	irq_install();
	
}

/* A simple wrapper that adds a function pointer to the IRQ array. */
void irq_install_handler(int irq, void (*handler)(struct int_frame* r)) {
	irq_routines[irq] = handler;
}

/* A simple wrapper that unlinks a function pointer, rendering the IRQ unused. */
void irq_uninstall_handler(int irq) {
	irq_routines[irq] = 0;
}

/* 
	Since the PIC starts with irq values that overlap with the CPU default ISR
	lines, it's necessary to remap the PIC.

	Doing this also means that we have to completely reinitialize the PIC, which
	is why this is so long.
 */

void irq_remap() {
	/* 0x20 is the Master PIC,
	   0xA0 is the Slave PIC. */
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);
}

/* A handler function to perform all of the required steps in one function call. 
	TODO: move all Descriptor Table things to a single callable function.
*/
void irq_install() {
	irq_remap();
	idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
	idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
	idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
	idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
	idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
	idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
	idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
	idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
	idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
	idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
	idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
	idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
	idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
	idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
	idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
	idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
}


/* All of the ISR routines call this function for now.
   ! This function is NOT leaf, and it might clobber the stack.
   ! Be careful! 
*/
void isr_common(struct int_frame* r, size_t exception) {
	/* We only have the capacity to handle 32 exceptions. This is a limitation of the CPU. */
	if(exception < 32) {
		/* exception_messages is an array defined in descriptor_tables.h */
		serial_print(0x3F8, exception_messages[exception]);
		serial_print(0x3F8, " Exception.\r\n");
		panic(exception_messages[exception]);
	}
}

/* The common handler for exceptions that throw error codes, which give us useful insight
	into what went wrong. In pure Curle style, though, we just ignore the error code. */
void isr_error_common(struct int_frame* r, size_t exception, size_t error) {

	if(exception < 32) {
		serial_print(0x3F8, exception_messages[exception]);
		serial_printf(0x3F8, " Exception. Context given: %d\r\n", error);
		panic(exception_messages[exception]);
	}

}

/* Likewise, this function is common to all IRQ handlers. It calls the assigned routine, 
	which was set up earlier by irq_install.*/
void irq_common(struct int_frame* r, size_t interrupt) {
	void (*handler)(struct int_frame* r);
	serial_print(0x3F8, "[INFO] Received IRQ: " + interrupt);
	/* We set all uninitialized routines to 0, so the if(handler) check here allows us to
		safely tell whether we've actually got something for this IRQ. */
	handler = irq_routines[interrupt];
	if(handler) 
		handler(r);
	
	/* The Slave PIC must be told it's been read in order to receive another 8+ IRQ. */
	if(interrupt > 7)
		outb(0xA0, 0x20);
	
	/* In either case, we tell the Master PIC it's been read to receive any IRQ. */
	outb(0x20, 0x20);
}
