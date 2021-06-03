/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/

/*
 * Sync main..
 *
 * Going off the back of Syncboot, the UEFI bootloader which currently only supports x86_64,
 * this kernel is to be rewritten. The kernel is started in Long Mode, with interrupts enabled.
 *
 * Therefore, most of the checks are no longer needed, as they are performed by Syncboot for us.
 * So, this becomes an exercise in time saving.
 *
 *
 */
 
//#include <stdio.h>
#include <kernel.h>

void gdb_end() {} /* GDB Debugging stump */

static size_t time = 0;

int kernel_main(FILELOADER_PARAMS* FLOP) {
	/* The kernel is started in 64-bit Long Mode by Syncboot. */

	/* PrepareSystem just initializes all hardware features and gets the system ready to execute every function.
	/* Most of this is just preparing the interrupts system, but there is a bit of messing with AVX features, which are used extensively in drawing graphics. */


	PrepareSystem(FLOP);

	gdb_end(); /* The first important step. Waypoint it for gdb debugging. */
	
	return 0;
}

