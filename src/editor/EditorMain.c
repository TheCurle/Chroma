#include <kernel/chroma.h>
#include <kernel/video/draw.h>
#include <editor/main.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

/**
 * Contains startup and setup routines for the Chroma Editor.
 */
static int KernelID;

void StartEditor(int callbackID) {
    KernelID = callbackID;

    struct EditorLayout layout = (struct EditorLayout) {0};
    layout.ScreenHeight = PrintInfo.screenHeight;
    layout.ScreenWidth = PrintInfo.screenWidth;
    layout.HeaderHeight = layout.ScreenHeight / 100 * 3;

    SetForegroundColor(0x000084);
    FillScreen();

    SetForegroundColor(0x00BBBBBB);

    DrawFilledRect(0, 0, PrintInfo.screenWidth, layout.HeaderHeight);

    for(;;) {}

}