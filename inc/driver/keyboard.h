#include <kernel/chroma.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char Char;
    char Scancode;
    bool Pressed;
} KeyboardData;

typedef void (*KeyboardCallback)(KeyboardData Frame);

extern KeyboardCallback KeyboardCallbacks[16];

int SetupKBCallback(void (*Handler)(KeyboardData Frame));

void UninstallKBCallback(int Index);

#ifdef  __cplusplus
}
#endif