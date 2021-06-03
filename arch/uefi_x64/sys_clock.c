/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/
 
/* This file provides an interface to
 * the hardware timer / RTC. Not much
 * more to be said about it. */

#include <kernel/utils.h>
#include <kernel.h>
#include <kernel/serial.h>
#include <kernel/descriptor_tables.h>

size_t timer_ticks = 0;
size_t flag = 0;
void timer_handler(struct int_frame* r) {
	gdb_end();
	timer_ticks++;
	
	if(timer_ticks % 18 == 0) {
		if(++flag % 2 == 0) {
			serial_print(0x3F8, "Tick.");
		} else {
			serial_print(0x3F8, "Tock.");
		}
	}

	if(timer_ticks > 18)
		timer_ticks = 0;
}

void timer_install() {
	irq_install_handler(0, &timer_handler);
}