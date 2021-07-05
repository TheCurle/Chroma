#include <kernel/chroma.h>
#include <kernel/system/driver/keyboard.h>
#include <kernel/video/draw.h>
#include <editor/main.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

/**
 * Contains startup and setup routines for the Chroma Editor.
 */
static KeyboardCallback KernelHandler;

void StartEditor(int callbackID) {
    KernelHandler = KeyboardCallbacks[callbackID];

    struct EditorLayout layout;
    layout.ScreenHeight = PrintInfo.screenHeight;
    layout.ScreenWidth = PrintInfo.screenWidth;
    layout.HeaderHeight = layout.ScreenHeight / 100 * 3;

    layout.TextBoxHeight = ((layout.ScreenHeight - layout.HeaderHeight) / 100) * 95;
    layout.TextBoxY = ((layout.ScreenHeight + layout.HeaderHeight) - layout.TextBoxHeight) / 2;

    layout.TextBoxWidth = (layout.ScreenWidth / 100) * 95;
    layout.TextBoxX = (layout.ScreenWidth - layout.TextBoxWidth) / 2;

    SetForegroundColor(0x000084);
    FillScreen();

    SetForegroundColor(0x00BBBBBB);
    DrawFilledRect(0, 0, PrintInfo.screenWidth, layout.HeaderHeight);
    DrawFilledRect(layout.TextBoxX, layout.TextBoxY, layout.TextBoxWidth, layout.TextBoxHeight);

    for(;;) {}

}