#include <kernel/chroma.h>
#include <kernel/video/bitmapfont.h>
#include <kernel/video/draw.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/**
 * This file contains all of the draw-to-screen routines.
 * It (currently; 23/08/20) handles the keyboard input test routine,
 *  moving the current character back and forth, up and down.
 *
 * See draw.h for more info.
 */


PRINTINFO PrintInfo = {0};

#define FONT bitfont_latin

static size_t strlen(const char* String) {
    size_t Len = 0;
    while(String[Len] != '\0') {
        Len++;
    }
    return Len;
}

void InitPrint() {
    PrintInfo.charHeight = 8;
    PrintInfo.charWidth = 8;
    PrintInfo.charFGColor = 0x00FFFFFF;
    PrintInfo.charHLColor = 0x00000000;
    PrintInfo.charBGColor = 0x00000000;

    PrintInfo.charScale = 1;

    PrintInfo.charPosX = 1;
    PrintInfo.charPosY = 1;
    PrintInfo.horizontalOffset = 1;

    PrintInfo.scrlMode = 0;

    PrintInfo.charsPerRow = bootldr.fb_width / (PrintInfo.charScale * PrintInfo.charWidth) - 4;
    PrintInfo.rowsPerScrn = bootldr.fb_height / (PrintInfo.charScale * PrintInfo.charHeight);
    SerialPrintf("[Print] A single character is %ux%u pixels.\r\n", PrintInfo.charScale * PrintInfo.charWidth, PrintInfo.charScale * PrintInfo.charHeight);
    SerialPrintf("[Print] The screen is %ux%u, meaning you can fit %ux%u characters on screen.\r\n", bootldr.fb_width, bootldr.fb_height, PrintInfo.charsPerRow, PrintInfo.rowsPerScrn);

    WriteString("Testing print\n");

}

void SetForegroundColor(uint32_t color) {
    PrintInfo.charFGColor = color;
}

uint32_t GetForegroundColor() {
    return PrintInfo.charFGColor;
}

void SetBackgroundColor(uint32_t color) {
    PrintInfo.charBGColor = color;
}

uint32_t GetBackgroundColor() {
    return PrintInfo.charBGColor;
}

static void DrawChar(const char character, size_t x, size_t y) {
    x = x + (PrintInfo.charPosX + PrintInfo.horizontalOffset) * (PrintInfo.charScale * PrintInfo.charWidth);
    y = y + PrintInfo.charPosY * (PrintInfo.charScale * PrintInfo.charHeight);

    uint32_t Y = PrintInfo.charWidth >> 3, X = 0;

    if((PrintInfo.charWidth & 0x7) != 0) {
        Y++;
    }

    for(uint32_t Row = 0; Row < PrintInfo.charHeight; Row++) {
        for(uint32_t Bit = 0; Bit < PrintInfo.charWidth; Bit++) {
            if ( ((Bit & 0x7) == 0) && (Bit > 0)) {
                X++;
            }

            // This one is crazy. Stick with me.
            for(uint32_t ScaleY = 0; ScaleY < PrintInfo.charScale; ScaleY++) { // Take care of the scale height
                for(uint32_t ScaleX = 0; ScaleX < PrintInfo.charScale; ScaleX++) { // And the scale width
                    size_t offset = ((y * bootldr.fb_width + x) +  // Calculate the offset from the framebuffer
                            PrintInfo.charScale * (Row * bootldr.fb_width + Bit) + // With the appropriate scale
                            (ScaleY * bootldr.fb_width + ScaleX) +  // In X and Y
                            PrintInfo.charScale * 1 * PrintInfo.charWidth) - 10;  // With some offset to start at 0

                    // Check the bit in the bitmap, if it's solid..
                    if((FONT[(int)character][Row * Y + X] >> (Bit & 0x7)) & 1) {
                        *(uint32_t* )(&fb + offset * 4) //use it to set the correct pixel on the screen
                            = PrintInfo.charFGColor; // In the set foreground color
                    } else { // otherwise,
                        *(uint32_t*)(&fb + offset *4)
                                = PrintInfo.charBGColor; // set the background color.
                    }
                }
            }
        }
    }
}

void DrawPixel(size_t x, size_t y) {
    if(x > bootldr.fb_width) {
        DrawPixel(x - bootldr.fb_width, y + (PrintInfo.charHeight * PrintInfo.charScale));
    } else if(y > bootldr.fb_height) {
        DrawPixel(x, y - bootldr.fb_height);
    } else {
        *((uint32_t*) (&fb + (y * bootldr.fb_width + x) * 4)) = PrintInfo.charFGColor;
    }
}

void FillScreen(uint32_t color) {
    uint32_t currentColor = GetForegroundColor();
    SetForegroundColor(color);
    for(size_t y = 0; y < bootldr.fb_height; y++) {
        for(size_t x = 0; x < bootldr.fb_width; x++) {
            DrawPixel(x, y);
        }
    }
    SetForegroundColor(currentColor);
}

static void ProgressCursorS(size_t Steps) {
    if(PrintInfo.charPosX + Steps > PrintInfo.charsPerRow) {
        PrintInfo.charPosX = 0;
        size_t Rows = Steps / PrintInfo.charsPerRow;
        if(PrintInfo.charPosY + Rows > PrintInfo.rowsPerScrn) {
            size_t RowsLeft = PrintInfo.rowsPerScrn - PrintInfo.charPosY;
            ProgressCursorS((Rows - RowsLeft) * PrintInfo.charsPerRow);
        } else {
            PrintInfo.charPosY++;
        }
    } else {
        PrintInfo.charPosX += Steps;
    }
}

static void Newline() {
    ProgressCursorS(PrintInfo.charsPerRow);
}

static void ProgressCursor() {
    ProgressCursorS(1);
}

static void Backspace() {

    SerialPrintf("Backspacing from %d to %d\r\n", PrintInfo.charPosX, PrintInfo.charPosX - 1);
    if(PrintInfo.charPosX - 1 <= 0) {
        if(PrintInfo.charPosY - 1 <= 0) {
            PrintInfo.charPosY = 0;
        } else {
            PrintInfo.charPosY--;
        }
        PrintInfo.charPosX = 0;
    } else {
        PrintInfo.charPosX -= 1;
    }
}

void WriteChar(const char character) {
    // TODO: Color codes!
    switch(character) {
        case '\b':
            Backspace();
            DrawChar((char) 32, PrintInfo.charPosX, PrintInfo.charPosY);
            break;
        case '\r':
            PrintInfo.charPosX = 0;
            break;
        case '\n':
            Newline();
            break;
        case '\t':
            ProgressCursorS(4);
            break;
        default:
            DrawChar(character, PrintInfo.charPosX, PrintInfo.charPosY);
            ProgressCursor();
            break;
    }

}

void WriteString(const char* string) {
    for(unsigned int i = 0; i < strlen(string); i++) {
        WriteChar(string[i]);
    }
}

// ! Deprecated.
// TODO: Fix this to work with arbitrary length and offset.
void WriteStringWithFont(const char *inChar) {
    psf_t *font = (psf_t*) &_binary_src_assets_font_psf_start;

    unsigned int drawX, drawY, kx = 0, fontLine, bitMask, offset;

    const unsigned int bytesPerLine = ( font -> glyphWidth + 7 ) / 8;

    while(*inChar) {
        unsigned char *glyph =
            (unsigned char*) &_binary_src_assets_font_psf_start
            + font->headerSize
            + (*inChar > 0 && *inChar < (int)font->numGlyphs ? *inChar : 0) *
            font->glyphSize;


        offset = (kx * (font->glyphWidth + 1) * 4);

        for( drawY = 0; drawY < font->glyphHeight ; drawY++) {
            fontLine = offset;
            bitMask = 1 << (font->glyphWidth - 1);

            for( drawX = 0; drawX < font->glyphWidth; drawX++) {

                *((uint32_t*)((uint64_t) &fb + fontLine)) =
                    ((int) *glyph) & (bitMask) ? 0xFFFFFF : 0;
                bitMask >>= 1;
                fontLine += 4;

            }

            *((uint32_t*)((uint64_t) &fb + fontLine)) = 0;
            glyph += bytesPerLine;
            offset += bootldr.fb_scanline;
        }

        inChar++; kx++;
    }
}
