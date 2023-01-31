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


PRINTINFO PrintInfo;

#define FONT bitfont_latin

void InitPrint() {
    PrintInfo.charHeight = 8;
    PrintInfo.charWidth = 8;
    PrintInfo.charFGColor = 0x00FFFFFF;
    PrintInfo.charHLColor = 0x00000000;
    PrintInfo.charBGColor = 0x00000000;

    PrintInfo.charScale = 1;

    PrintInfo.charPosX = 0;
    PrintInfo.charPosY = 0;
    PrintInfo.horizontalOffset = 1;

    PrintInfo.scrlMode = 0;

    PrintInfo.screenWidth = bootldr.fb_width;
    PrintInfo.screenHeight = bootldr.fb_height;

    PrintInfo.charsPerRow = bootldr.fb_width / (PrintInfo.charScale * PrintInfo.charWidth);
    PrintInfo.rowsPerScrn = bootldr.fb_height / (PrintInfo.charScale * PrintInfo.charHeight);
    SerialPrintf("[Print] A single character is %ux%u pixels.\r\n", PrintInfo.charScale * PrintInfo.charWidth,
                 PrintInfo.charScale * PrintInfo.charHeight);
    SerialPrintf("[Print] The screen is %ux%u, meaning you can fit %ux%u characters on screen.\r\n", bootldr.fb_width,
                 bootldr.fb_height, PrintInfo.charsPerRow, PrintInfo.rowsPerScrn);

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

    if ((PrintInfo.charWidth & 0x7) != 0) {
        Y++;
    }

    for (uint32_t Row = 0; Row < PrintInfo.charHeight; Row++) {
        for (uint32_t Bit = 0; Bit < PrintInfo.charWidth; Bit++) {
            if (((Bit & 0x7) == 0) && (Bit > 0)) {
                X++;
            }

            // This one is crazy. Stick with me.
            for (uint32_t ScaleY = 0; ScaleY < PrintInfo.charScale; ScaleY++) { // Take care of the scale height
                for (uint32_t ScaleX = 0; ScaleX < PrintInfo.charScale; ScaleX++) { // And the scale width
                    size_t offset = ((y * bootldr.fb_width + x) +  // Calculate the offset from the framebuffer
                                     PrintInfo.charScale * (Row * bootldr.fb_width + Bit) +
                                     // With the appropriate scale
                                     (ScaleY * bootldr.fb_width + ScaleX) +  // In X and Y
                                     PrintInfo.charScale * 1 * PrintInfo.charWidth) -
                                    10;  // With some offset to start at 0

                    // Check the bit in the bitmap, if it's solid..
                    if ((FONT[(int) character][Row * Y + X] >> (Bit & 0x7)) & 1) {
                        *(uint32_t*) (&fb + offset * 4) //use it to set the correct pixel on the screen
                                = PrintInfo.charFGColor; // In the set foreground color
                    } else { // otherwise,
                        *(uint32_t*) (&fb + offset * 4)
                                = PrintInfo.charBGColor; // set the background color.
                    }
                }
            }
        }
    }
}

inline void DrawPixel(size_t x, size_t y) {
    *((uint32_t*) (&fb + (y * bootldr.fb_width + x) * 4)) = PrintInfo.charFGColor;
}

void FillScreen(uint32_t color) {
    for (uint32_t pixel = 0; pixel < bootldr.fb_width * bootldr.fb_height; pixel++) {
        ((uint32_t*) &fb)[pixel] = color;
    }
}

static void Newline() {
    // One row down, unless it goes off the screen, in which case start from the top.
    PrintInfo.charPosY = PrintInfo.charPosY + 1 > PrintInfo.rowsPerScrn ? 0 : PrintInfo.charPosY + 1;
}

static void ProgressCursorS(size_t Steps) {
    if (PrintInfo.charPosX + Steps > PrintInfo.charsPerRow) {
        PrintInfo.charPosX = 0;
        size_t Rows = Steps / PrintInfo.charsPerRow;
        if (PrintInfo.charPosY + Rows > PrintInfo.rowsPerScrn) {
            size_t RowsLeft = PrintInfo.rowsPerScrn - PrintInfo.charPosY;
            ProgressCursorS(RowsLeft + 1);
            Newline();
        } else {
            PrintInfo.charPosY += Rows;
        }
    } else {
        PrintInfo.charPosX += Steps;
    }
}


static void ProgressCursor() {
    ProgressCursorS(1);
}

static void Backspace() {

    SerialPrintf("Backspacing from %d to %d\r\n", PrintInfo.charPosX, PrintInfo.charPosX - 1);
    if (PrintInfo.charPosX - 1 <= 0) {
        if (PrintInfo.charPosY - 1 <= 0) {
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
    switch (character) {
        case '\b':
            Backspace();
            DrawChar((char) 32, PrintInfo.charPosX, PrintInfo.charPosY);
            break;
        case '\r':
            PrintInfo.charPosX = 0;
            break;
        case '\n':
            PrintInfo.charPosX = 0;
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
    for (unsigned int i = 0; i < strlen(string); i++) {
        WriteChar(string[i]);
    }
}

// ! Deprecated.
// TODO: Fix this to work with arbitrary length and offset.
void WriteStringWithFont(const char* inChar) {
    psf_t* font = (psf_t*) &_binary_src_assets_font_psf_start;

    unsigned int drawX, drawY, kx = 0, fontLine, bitMask, offset;

    const unsigned int bytesPerLine = (font->glyphWidth + 7) / 8;

    while (*inChar) {
        unsigned char* glyph =
                (unsigned char*) &_binary_src_assets_font_psf_start
                + font->headerSize
                + (*inChar > 0 && *inChar < (int) font->numGlyphs ? *inChar : 0) *
                  font->glyphSize;


        offset = (kx * (font->glyphWidth + 1) * 4);

        for (drawY = 0; drawY < font->glyphHeight; drawY++) {
            fontLine = offset;
            bitMask = 1 << (font->glyphWidth - 1);

            for (drawX = 0; drawX < font->glyphWidth; drawX++) {

                *((uint32_t*) ((uint64_t) &fb + fontLine)) =
                        ((int) *glyph) & (bitMask) ? 0xFFFFFF : 0;
                bitMask >>= 1;
                fontLine += 4;

            }

            *((uint32_t*) ((uint64_t) &fb + fontLine)) = 0;
            glyph += bytesPerLine;
            offset += bootldr.fb_scanline;
        }

        inChar++;
        kx++;
    }
}

/********************************** Geometry Rendering **********************************/
/**
 * Implementation borrowed with <3 from Adafruit: https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.cpp
 * TODO: Reimplement in Helix.
 *
 */

/******* Internal *******/

inline void _swap_size_t(size_t a, size_t b) {
    size_t t = a;
    a = b;
    b = t;
}

inline int abs(int x) {
    return x < 0 ? -x : x;
}

void DrawLineInternal(size_t x0, size_t y0, size_t x1, size_t y1) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        _swap_size_t(x0, y0);
        _swap_size_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_size_t(x0, x1);
        _swap_size_t(y0, y1);
    }

    size_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int err = dx / 2;
    size_t ystep;

    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    for (; x0 <= x1; x0++) {
        if (steep)
            DrawPixel(y0, x0);
        else
            DrawPixel(x0, y0);

        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void DrawHorizontalLine(size_t x, size_t y, size_t w) {
    DrawLineInternal(x, y, x + w - 1, y);
}

void DrawVerticalLine(size_t x, size_t y, size_t h) {
    DrawLineInternal(x, y, x, y + h - 1);
}

void DrawFilledCircleInternal(size_t centerX, size_t centerY, size_t radius, char cornerMask, size_t offset) {
    int f = 1 - radius;
    size_t ddF_x = 1;
    size_t ddF_y = -2 * radius;
    size_t x = 0;
    size_t y = radius;
    size_t px = x;
    size_t py = y;

    offset++;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (x < (y + 1)) {
            if (cornerMask & 1)
                DrawVerticalLine(centerX + x, centerY - y, 2 * y + offset);
            if (cornerMask & 2)
                DrawVerticalLine(centerX - x, centerY - y, 2 * y + offset);
        }

        if (y != py) {
            if (cornerMask & 1)
                DrawVerticalLine(centerX + py, centerY - px, 2 * px + offset);
            if (cornerMask & 2)
                DrawVerticalLine(centerX - py, centerY - px, 2 * px + offset);
            py = y;
        }
        px = x;
    }
}

/******* Public *******/

/** TODO: Move all of this into static DrawUtils:: functions */

void DrawFilledRect(Frame* f, size_t x, size_t y, size_t width, size_t height, uint32_t color) {
    uint32_t currentX;
    uint32_t maxX = x + width;
    uint32_t maxY = y + height;

    if (maxX > f->width)
        maxX = f->width;
    if (maxY > f->height)
        maxY = f->height;

    for (; y < maxY; y++)
        for (currentX = x; currentX < maxX; currentX++)
            f->buffer[y * f->width + currentX] = color;
}

void DrawLineRect(Frame* f, size_t x, size_t y, size_t width, size_t height, size_t thickness, uint32_t color) {
    UNUSED(thickness); UNUSED(f); UNUSED(color);
    DrawHorizontalLine(x, y, width);
    DrawHorizontalLine(x, y + height - 1, width);
    DrawVerticalLine(x, y, height);
    DrawVerticalLine(x + width - 1, y, height);
}

void DrawFilledRoundedRect(Frame* f, size_t x, size_t y, size_t width, size_t height, size_t radius, uint32_t color) {
    size_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
    if (radius > max_radius)
        radius = max_radius;
    DrawFilledRect(f, x + radius, y, width - 2 * radius, height, color);
    // draw four corners
    DrawFilledCircleInternal(x + width - radius - 1, y + radius, radius, 1, height - 2 * radius - 1);
    DrawFilledCircleInternal(x + radius, y + radius, radius, 2, height - 2 * radius - 1);
}

void DrawLineRoundedRect(Frame* f, size_t x, size_t y, size_t width, size_t height, size_t radius, uint32_t color) {
    UNUSED(color);
    size_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
    if (radius > max_radius)
        radius = max_radius;
    DrawHorizontalLine(x + radius, y, width - 2 * radius);         // Top
    DrawHorizontalLine(x + radius, y + height - 1, width - 2 * radius); // Bottom
    DrawVerticalLine(x, y + radius, height - 2 * radius);         // Left
    DrawVerticalLine(x + width - 1, y + radius, height - 2 * radius); // Right
    // draw four corners
    DrawLineCircleCorners(f, x + radius, y + radius, radius, 1);
    DrawLineCircleCorners(f, x + width - radius - 1, y + radius, radius, 2);
    DrawLineCircleCorners(f, x + width - radius - 1, y + height - radius - 1, radius, 4);
    DrawLineCircleCorners(f, x + radius, y + height - radius - 1, radius, 8);
}

void DrawLine(size_t x0, size_t y0, size_t x1, size_t y1) {
    if (x0 == x1) {
        if (y0 > y1)
            _swap_size_t(y0, y1);

        DrawVerticalLine(x0, y0, y1 - y0 + 1);
    } else if (y0 == y1) {
        if (x0 > x1)
            _swap_size_t(x0, x1);

        DrawHorizontalLine(x0, y0, x1 - x0 + 1);
    } else {
        DrawLineInternal(x0, y0, x1, y1);
    }
}

void DrawCircle(size_t centerX, size_t centerY, size_t radius) {
    int f = 1 - radius;
    size_t ddF_x = 1;
    size_t ddF_y = -2 * radius;
    size_t x = 0;
    size_t y = radius;

    DrawPixel(centerX, centerY + radius);
    DrawPixel(centerX, centerY - radius);
    DrawPixel(centerX + radius, centerY);
    DrawPixel(centerX - radius, centerY);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        DrawPixel(centerX + x, centerY + y);
        DrawPixel(centerX - x, centerY + y);
        DrawPixel(centerX + x, centerY - y);
        DrawPixel(centerX - x, centerY - y);
        DrawPixel(centerX + y, centerY + x);
        DrawPixel(centerX - y, centerY + x);
        DrawPixel(centerX + y, centerY - x);
        DrawPixel(centerX - y, centerY - x);
    }
}

void DrawLineCircleCorners(Frame* fr, size_t centerX, size_t centerY, size_t radius, char cornerMask) {
    UNUSED(fr);
    int f = 1 - radius;
    size_t ddF_x = 1;
    size_t ddF_y = -2 * radius;
    size_t x = 0;
    size_t y = radius;

    DrawPixel(centerX, centerY + radius);
    DrawPixel(centerX, centerY - radius);
    DrawPixel(centerX + radius, centerY);
    DrawPixel(centerX - radius, centerY);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornerMask & 0x4) {
            DrawPixel(centerX + x, centerY + y);
            DrawPixel(centerX + y, centerY + x);
        }

        if (cornerMask & 0x2) {
            DrawPixel(centerX + x, centerY - y);
            DrawPixel(centerX - y, centerY - x);
        }

        if (cornerMask & 0x8) {
            DrawPixel(centerX - y, centerY + x);
            DrawPixel(centerX - x, centerY + y);
        }

        if (cornerMask & 0x1) {
            DrawPixel(centerX - y, centerY - x);
            DrawPixel(centerX - x, centerY - y);
        }
    }
}

void DrawFilledCircle(size_t centerX, size_t centerY, size_t radius) {
    DrawVerticalLine(centerX, centerY - radius, 2 * radius + 1);
    DrawFilledCircleInternal(centerX, centerY, radius, 3, 0);
}
