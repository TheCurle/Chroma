/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/
 
 /* This file provides an interface to the serial port.
  * Most emulators allow the serial port to be saved into
  * a text buffer, or a file, but this code should work on 
  * hardware with an actual serial port and monitor. */

 /* After the development of Syncboot, this becomes a little bit harder.
  * Some testing will be needed to know for certain whether this will work.
  * Until then, this is to be put on hold. 21/07/19 - Curle */

#include <kernel.h>

#define SERIAL_DATA(base)           	(base)
#define SERIAL_DLAB(base)				(base + 1)
#define SERIAL_FIFO(base)   			(base + 2)
#define SERIAL_LINE(base)  				(base + 3)
#define SERIAL_MODEM(base)  			(base + 4)
#define SERIAL_LINE_STATUS(base)    	(base + 5)
#define COM1			                0x3F8


/**   serialSetBaudrate:
  *   Sets the speed that data is sent via the serial port.
  *   The default speed is 115200 bits/s.
  *   The argument given is the divisor of the number.
  *   Hence, the new speed becomes (115200/divisor) bits/s.
  *
  *   @param divisor  The new divisor for the baud rate.
  */

void serialSetBaudrate() {
	WritePort(SERIAL_LINE(COM1), 0x80, 1);
		 
	WritePort(SERIAL_DATA(COM1), 0x03, 1);
	
	WritePort(SERIAL_DLAB(COM1), 0x00, 1);
		
}

/**   serialConfigure:
  *   Sets a specific data byteset in the Serial hardware.
  *   Required for expected operation.
  */

void serialConfigure() {
   /* Bit:        | 7 | 6 | 5 4 3  | 2 | 1 0 |
    * Content:    | d | b | parity | s | dl  |
	* Value:      | 0 | 0 | 0 0 0  | 0 | 1 1 | = 0x03
	*/
	WritePort(SERIAL_LINE(COM1), 0x03, 1);
}

/**   serialConfigureBuffers:
  *   Enables FIFO (First In First Out) on the Serial line.
  *   It clears both the receive and transmit queues.
  *   It uses a 14 byte wide queue.
  *
  */
  
void serialConfigureBuffers() {
   /* Bit:        | 7 6 | 5  | 4 |  3  |  2  |  1  | 0 |
	* Content:    | 1v1 | bs | r | dma | clt | clr | e |
	* Value:      | 1 1 | 0  | 0 |  0  |  1  |  1  | 1 | = 0xC7
	*/
	
	WritePort(SERIAL_FIFO(COM1), 0xC7, 1);
}

/**   serialConfigureModem:
  */
  
void serialConfigureModem() {
   /* Bit:        | 7 | 6 | 5  | 4  |  3  |  2  |  1  |  0  |
    * Content:    | r | r | af | lb | ao2 | ao1 | rts | dtr |
	* Value:      | 0 | 0 | 0  | 0  |  0  |  0  |  1  |  1  | = 0x03
	*/
	
	WritePort(SERIAL_MODEM(COM1), 0x03, 1);
}

/**   serialCheck:
  *   Checks whether the Transmit FIFO Queue is empty on the given port.
  *
  *   @return  0  If the queue is not empty.
  *            1  If the queue is empty.
  */
  
int serialCheck() {
	return ReadPort(SERIAL_LINE_STATUS(COM1), 1) & 0x20;
}

/**   serial_write:
  *   Prints a single character to the serial port.
  *   Does nothing fancy.
  *   @param data The character to write.
  */

void serialWrite(const char chr) {
	//Hang until we have access to the COM port.
	while(!serialCheck());
	WritePort(COM1, chr, 1);
}

/**   serialPrint:
  *   Prints a string to the serial port.
  *   Does not support ANSI codes, or color.
  *
  *   @param data The string to write.
  */
 
void serialPrint(const char* data) {
	for(size_t i = 0; i < strlen(data); i++) {
		serialWrite(data[i]);
	}
}

/**   serialPrintf:
  *   A printf-style function for the serial port.
  *   
  *   @param format  The base string - used to substitute the succeding values.
  *   @param ...     The substitutions.
  */
  
void serialPrintf(const char* format, ...) {
	uint32_t storage; //To hold temporary variables
	char stringstore[10] = {0}; //To convert ints to strings.
	va_list list;
	va_start(list, format);
	
	for(size_t i = 0; i < strlen(format); i++) {
		//Check for the format specifier
		if(format[i] == '%') {
			//Check for the other specs
			if(format[i+1] == 'd') {
				
				storage = va_arg(list, int);
				inttoa(storage, stringstore);
				serialPrint(stringstore);
				zeroString(stringstore);
				i += 2;
				
			} else if(format[i+1] == 'x') {
				
				storage = va_arg(list, int);
				itoh(storage, stringstore);
				serialPrint(stringstore);
				zeroString(stringstore);
				i += 2;
				
			} else if(format[i+1] == 's') {
				
				serialPrint(va_arg(list, char*));
				i += 2;
				
			} else {
				serialPrint("ERROR: Attempting to parse unknown format string.");
				return;
			}
		} else {
			serialWrite(format[i]);
			i++;
		}
	}
	va_end(list); //Free the list
}

void serialInit() {
	// Disable interrupts
	WritePort(SERIAL_DLAB(COM1), 0x00, 1);
	
	// Set baud rate divisor to 3
	serialSetBaudrate();
	// 8 bits, no parity, one stop bit.
	serialConfigure();
	// Enable FIFO and clear buffers.
	serialConfigureBuffers();
	// Enable IRQs, RTS/DSR set.
	serialConfigureModem();
}
