#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/************************
 *** Team Kitty, 2021 ***
 ***    Lainlib       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char* String) {
    size_t Len = 0;
    while(String[Len] != '\0') {
        Len++;
    }
    return Len;
}

bool strcmp(char* a, const char* b) {
    size_t aI = 0, bI = 0;
    while(true) {
        if(b[bI] == '\0') return true;
        if(a[aI] != b[bI]) return false;
        aI++;
        bI++;
    }
}

#ifdef  __cplusplus
}
#endif