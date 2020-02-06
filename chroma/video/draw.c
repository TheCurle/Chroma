#include <kernel/chroma.h>
#include <kernel/video/bitmapfont.h>

#define FONT bitfont_latin

static size_t strlen(const char* String) {
    size_t Len = 0;
    while(String[Len] != '\0') {
        Len++;
    }
    return Len;
}

typedef struct {
    uint32_t    charHeight;
    uint32_t    charWidth;
    uint32_t    charFGColor;
    uint32_t    charHLColor;
    uint32_t    charBGColor;
    uint32_t    minX;
    uint32_t    minY;
    uint32_t    charScale;
    size_t      charPosX;
    size_t      charPosY;
    size_t      charsPerRow;
    size_t      rowsPerScrn;
    uint32_t    scrlMode;
} PRINTINFO;

PRINTINFO PrintInfo = {0};

void InitPrint() {
    PrintInfo.charHeight = 8;
    PrintInfo.charWidth = 8;
    PrintInfo.charFGColor = 0x00FFFFFF;
    PrintInfo.charHLColor = 0x00000000;
    PrintInfo.charBGColor = 0x00000000;

    PrintInfo.charScale = 2;

    PrintInfo.charPosX = 0;
    PrintInfo.charPosY = 0;

    PrintInfo.scrlMode = 0;

    PrintInfo.charsPerRow = bootldr.fb_width / (PrintInfo.charScale * PrintInfo.charWidth) - 5;
    PrintInfo.rowsPerScrn = bootldr.fb_height / (PrintInfo.charScale * PrintInfo.charHeight);
    SerialPrintf("A single character is %ux%u pixels.\r\n", PrintInfo.charScale * PrintInfo.charWidth, PrintInfo.charScale * PrintInfo.charHeight);
    SerialPrintf("The screen is %ux%u, meaning you can fit %ux%u characters on screen.\r\n", bootldr.fb_width, bootldr.fb_height, PrintInfo.charsPerRow, PrintInfo.rowsPerScrn);

}

static void DrawChar(const char character, size_t x, size_t y) {
    x = x + PrintInfo.charPosX * (PrintInfo.charScale * PrintInfo.charWidth);
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

            if((FONT[character][Row * Y + X] >> (Bit & 0x7)) & 1) {
                for(uint32_t ScaleY = 0; ScaleY < PrintInfo.charScale; ScaleY++) {
                    for(uint32_t ScaleX = 0; ScaleX < PrintInfo.charScale; ScaleX++) {
                    
                        size_t offset = ((y * bootldr.fb_width + x) + 
                                PrintInfo.charScale * (Row * bootldr.fb_width + Bit) + 
                                (ScaleY * bootldr.fb_width + ScaleX) + 
                                PrintInfo.charScale * 1 * PrintInfo.charWidth) - 10;  
                        
                        // This one is crazy. Stick with me.
                        *(uint32_t* )(&fb + offset * 4)// Offset from the framebuffer, find the pixel..
                        // and find which column we need to be in. Multiply by 4 to navigate the 4bpp array,
                                = PrintInfo.charFGColor; // And set the color of the pixel.
                    }
                }
            } else {
                // We need to draw the pixel transparently..
                for(uint32_t ScaleY = 0; ScaleY < PrintInfo.charScale; ScaleY++) {
                    for(uint32_t ScaleX = 0; ScaleX < PrintInfo.charScale; ScaleX++) {
                        if(PrintInfo.charHLColor != 0xFF000000) {
                            size_t offset = ((y * bootldr.fb_width + x) + 
                                    PrintInfo.charScale * (Row * bootldr.fb_width + Bit) + 
                                    (ScaleY * bootldr.fb_width + ScaleX) + 
                                    PrintInfo.charScale * 1 * PrintInfo.charWidth) - 10;
                            if( y == 0 && x == 0){
                            SerialPrintf("Writing first pixel at %x\r\n", offset);
                            }

                          *(uint32_t*)(&fb + offset *4)
                                    = PrintInfo.charHLColor;

                        }
                    }
                }
            }
        }
    }




    /*
    uint32_t Y = PrintInfo.charWidth >> 3, X = 0;


    if((PrintInfo.charWidth & 0x7) != 0) {
        Y++;
    }

    for(size_t CharRow = 0; CharRow < PrintInfo.charHeight; CharRow++) {
        for(size_t CharPixel = 0; CharPixel < PrintInfo.charWidth; CharPixel++) {
            if( CharPixel && (CharPixel & 0x7) == 0) {
                X++;
            }

            for(size_t ScaleX = 0; ScaleX < PrintInfo.charScale; ScaleX++) {
                for(size_t ScaleY = 0; ScaleY < PrintInfo.charScale; ScaleY++) {

                    if(FONT[character][CharRow * Y + X] >> (CharRow & 0x7) & 1) {
                    
                        size_t offset = (y * bootldr.fb_width + x) +
                            (PrintInfo.charScale * (CharRow * bootldr.fb_width + CharPixel)) +
                            (ScaleY * (bootldr.fb_width + ScaleX));


                            *((uint32_t*) (&fb + offset * 4))
                                = PrintInfo.charFGColor;
                        //DrawPixel(x * PrintInfo.charWidth + X, Y + y + PrintInfo.charHeight, 0x00FF0000); 
                    }
                }
            }
        }
    }

    uint32_t Y = PrintInfo.charWidth >> 3, X = 0;


    if((PrintInfo.charWidth & 0x7) != 0) {
        Y++;
    }

    for(uint32_t Row = 0; Row < PrintInfo.charHeight; Row++) {
        for(uint32_t Bit = 0; Bit < PrintInfo.charWidth; Bit++) {
            if( ((Bit & 0x7) == 0) && (Bit > 0))
                X++;

            for(uint32_t ScaleY = 0; ScaleY < PrintInfo.charScale; ScaleY++) {
                for(uint32_t ScaleX = 0; ScaleX < PrintInfo.charScale; ScaleX++) {
                    if((FONT[character][Row * Y + X] >> (Bit & 0x7)) & 1) {
                        size_t offset = (y * bootldr.fb_width + x) +
                            (PrintInfo.charScale * (Row * bootldr.fb_width + Bit)) +
                            (ScaleY * (bootldr.fb_width + ScaleX));
                        SerialPrintf("Drawing pixel at offset %x, at y %d, x %d, row %d, bit %d with scaley %d and scalex %d. charScale %d and charWidth %d, making a charLength %d.\r\n", offset, y, x, Row, Bit, ScaleX, ScaleY, PrintInfo.charScale, PrintInfo.charWidth, PrintInfo.charScale * PrintInfo.charWidth);

                        *((uint32_t*) (&fb + offset * 4))
                            = PrintInfo.charFGColor;
                    } else {
                        if(PrintInfo.charHLColor != 0xFF000000) {
                            size_t offset = (y * bootldr.fb_width + x) +
                                (PrintInfo.charScale * (Row * bootldr.fb_width + Bit)) +
                                (ScaleY * (bootldr.fb_width + ScaleX));
                            SerialPrintf("Drawing pixel at offset %x\r\n", offset);

                            *((uint32_t*) (&fb + offset * 4))
                                = PrintInfo.charHLColor;
                        }
                    }
                }
            }
        }
    }

    */
}

void DrawPixel(uint32_t x, uint32_t y, uint32_t color) {
    if(x > bootldr.fb_width) {
        DrawPixel(x - bootldr.fb_width, y + (PrintInfo.charHeight * PrintInfo.charScale), color);
        //FillScreen(color);
    } else if(y > bootldr.fb_height) {
        DrawPixel(x, y - bootldr.fb_height, color);
    } else {
        *((uint32_t*) (&fb + (y * bootldr.fb_scanline + x) * 4)) = color;
        SerialPrintf("Drawing a pixel at %d, %d with color 0x%x\r\n", x, y, color);
    }
}

void FillScreen(uint32_t color) {
    for(size_t y = 0; y < bootldr.fb_height; y++) {
        for(size_t x = 0; x < bootldr.fb_width; x++) {
            DrawPixel(x, y, color);
        }
    }
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

void WriteChar(const char character) {

    //size_t y = PrintInfo.charPos / RowsWidth * (PrintInfo.charScale * PrintInfo.charHeight);
    //size_t x = (PrintInfo.charPos % RowsWidth) * (PrintInfo.charScale * PrintInfo.charWidth);

    switch(character) {
        case '\b':
            if(PrintInfo.charPosX - 1 == 0) {
                if(PrintInfo.charPosY - 1 == 0) {
                    PrintInfo.charPosY = 0;
                } else {
                    PrintInfo.charPosY--;
                }
            } else {
                PrintInfo.charPosX = 0;
            }
            break;
        case '\n':
            Newline();
            break;
        default:
            DrawChar(character, PrintInfo.charPosX, PrintInfo.charPosY);
            
            ProgressCursor();
            break;
    }

}



void WriteString(const char* string) {
    for(int i = 0; i < strlen(string); i++) {
        WriteChar(string[i]);
    }
}
