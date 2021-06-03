/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/
 
/* This file contains utility functions
 * for the kernel and OS to use. They 
 * will be naturally optimized through
 * the run of development, and thus are
 * to be used scarcely until a more
 * permanent solution can be found.
 */

#include <kernel.h>

/*
	Returns the length of a given string.
  * @param  string: String to count
  */
size_t strlen(const char* string) {
	size_t size = 0;
	while (string[size]) 
		size++;
	return size;
}

/* 
	Memory Copy. Required by GCC.
 *  @param dest: The destination in memory, to which the data will be copied.
 *  @param src:  The source of the data which will be copied.
 *  @param n:    The length of the copy, in byets.
 */
void memcpy(void* dest, void* src, size_t n) {
	char* src_c  = (char*)src;
	char* dest_c = (char*)dest;
	
	for(size_t i = 0; i < n; i++) {
		dest_c[i] = src_c[i];
	}
}

void* memmove (void *dest, const void *src, size_t len) {
  const char *s = (char *)src;
  char *d = (char *)dest;

  const char *nexts = s + len;
  char *nextd = d + len;

  if (d < s) {
    while (d != nextd) {
      *d++ = *s++;
    }
  }
  else {
    while (nextd != d) {
      *--nextd = *--nexts;
    }
  }
  return dest;
}


/*
	Memory Set. Required by GCC.
 *  @param src:  The data to be overwritten.
 *  @param chr:  The byte to overwrite the source with.
 *  @param n:    How many bytes to overwrite.
*/
void memset(void* src, int chr, size_t n) {
	uint8_t* ptr = src;
	while(n--) {
		*ptr++ = (uint8_t) chr;
	}
}

int memcmp (const void *str1, const void *str2, size_t count) {
  const unsigned char *s1 = (unsigned char *)str1;
  const unsigned char *s2 = (unsigned char *)str2;

  while (count-- > 0) {
    if (*s1++ != *s2++) {
      return s1[-1] < s2[-1] ? -1 : 1;
    }
  }
  return 0;
}


 /*
	Turns an integer into a C-str.
  * @note   Inefficient and unsafe.
  * @param  num: The number to convert
  * @param  string: The string to be written into.
  */

void inttoa(int num, char* string) {
	size_t i = 0; //Counter.
	// TODO: Convert this to a for-loop?
	int32_t calc = 0;
	bool negative = false;
	
	if(num == 0) {
		string[0] = '0';
		string[1] = '\0';
		return;
	}
	
	// TODO: Implement this as an abs() function?
	if(num < 0) {
		num = -num;
		string[i] = '-';
		i++;
	}
	
	while(num != 0) {
		// Overall this looks pretty confusing.
		// TODO: cleanup?
		calc = num % 10; // Gets the lowest digit,
		num = num / 10;  // Shifts the other digits down to prepare for the next iteration.
		calc += 48;      // Convert the number to an ASCII value
		
		string[i] = calc;// Set the current position in the string
		i++;			 // Increase for the next number.
	}
	
	// ! This code does weird things.
	// It serves an unknown purpose, and should not be used, but is left here just in case.
	/*
	for(size_t j = 0; j < i/2; j++) {
		calc = string[j];
		string[j] = string[i-j-1];
		string[j-i-1] = calc;
	}
	*/
}

 /*
	A strange version of the above function. Different way of doing the same thing?
  * @param  num: the number to convert. Must be positive
  * @retval The new string.
  */
char* itoc(size_t num) {
	char* result;
	size_t tmp_value;
	do {
		tmp_value = num;
		num /= 10;
		*result++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - num * 10)];
	}while (num);

	return result;
}


/* 
	Same as itoa, but for hexadecimal.
 *  @param num: Number to convert
 *  @param string: String pointer to put the converted number.
*/
void itoh(int num, char* string) {
	zeroString(string);
	
	uint8_t i = 8;
	uint8_t temp = 0;
	
	while(i != 0) {
		i--;
		temp = num & 0xF;
		num = num >> 4;
		
		if(temp >= 10) {
			temp += 7;
		}
		
		temp += 48;
		string[i] = temp;
	}
}

 /*
	Converts a C-str to an empty C-str. :)
  * @param  string: The string to empty.
  */

void zeroString(char* string) {
	size_t len = strlen(string);
	
	for(size_t i = 0; i < len + 1; i++) {
		string[i] = '\0';
	}
}


 /*
	The signal that everything has gone wrong.
	Equivalent to a linux kernel panic and a Windows Blue Screen.

	TODO: Needs to be re-engineered to give as much useful information as possible. Ideally also have a visual interface, XP installer style.

  * @param  cause: A string, telling the basic reason for the crash.
  */
void panic() {
	printf("Kernel Halted.");

	for(;;);

}
