/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/


/* ==================== Kernel Graphics ==================== */
// This file allows the kernel to draw to the framebuffer provided by Syncboot.
// It provides the setup, printing and drawing functions.
// Eventually this will be split up into printing and drawing to provide room for expansion.
// I'm thinking about making a 3D interface.

#include <kernel.h>

//#include <bitfont.h>

// Get it? Default Font
//         Defaunt
//         Defont
#define DEFONT bitfont_latin


void SetupPrinting(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU) {
    Print_Info.defaultGPU = GPU;
    Print_Info.charHeight = 8;
    Print_Info.charWidth = 8;

    Print_Info.charFGColor = 0x00FFFFFF;
    Print_Info.charBGColor = 0x00000000;
    Print_Info.charHLColor = 0x00000000;

    // per UEFI Spec 2.7 Errata A, (0,0) is always the top left in-bounds pixel.
    Print_Info.screenMinX = 0;
    Print_Info.screenMinY = 0;

    Print_Info.charScale = 1;
    Print_Info.cursorPos = 0;

    Print_Info.scrollMode = 0;
}

void WriteScaledFormatString(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                uint32_t Height,
                uint32_t Width,
                uint32_t FontColor,
                uint32_t HighlightColor,
                uint32_t x,
                uint32_t y,
                uint32_t Scale,
                const char* String,
                ...) {
    
    if(Height > GPU.Info->VerticalResolution || Width > GPU.Info->HorizontalResolution) {
        // Turn the screen red.
        // TODO: Write a function to turn the *borders* of the screen a certain color.
        FillScreen(GPU, 0x00FF00000);
    } else if (y > GPU.Info->VerticalResolution || x > GPU.Info->HorizontalResolution) {
        // Turn the screen green.
        FillScreen(GPU, 0x0000FF00);
    } else if ((y + Scale * Height) > GPU.Info->VerticalResolution || (x + Scale * Width) > GPU.Info->HorizontalResolution) {
        // Turn the screen blue.
        FillScreen(GPU, 0x000000FF);
    }

    // Take the ... arguments into a buffer and pass them to a reasonable function.

    va_list Args;
    va_start(Args, String);

    char Output[128] = {0};
    vsprintf(Output, String, Args);
    va_end(Args);

    WriteScaledString(GPU, Output, Height, Width, FontColor, HighlightColor, x, y, Scale);

}

void ResetScreen() {
    Print_Info.screenMinX = 0;
    Print_Info.screenMinY = 0;
    Print_Info.cursorPos = 0;

    ClearScreen(Print_Info.defaultGPU);
}

void ResetFillScreen() {
    ResetScreen();
    FillScreen(Print_Info.defaultGPU, Print_Info.charBGColor);
}

void ClearScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU) {
    FillScreen(GPU, 0x00000000);
}

void FillScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU, uint32_t Color) {
    Print_Info.charBGColor = Color;
    // The fastest way to do this is using AVX fills.
    // However, if AVX isn't supported, we'll need to fall back to filling the screen the old fashioned way.
    // The performance benefit by using AVX Parallel operations is huge.
    memsetAVX_By4Bytes( (EFI_PHYSICAL_ADDRESS* )GPU.FrameBufferBase, Color, GPU.Info->VerticalResolution * GPU.Info->PixelsPerScanline);

    // If AVX isn't supported, we can use this instead:

   /*
    for(size_t Row = 0; Row < GPU.Info->VerticalResolution; Row++) {
        for(size_t Column = 0; Column < GPU.Info->PixelsPerScanline - (GPU.Info->PixelsPerScanline - GPU.Info->HorizontalResolution); Column++) {
            *(uint32_t *)GPU.FrameBufferBase + 4 * (GPU.Info->PixelsPerScanline * Row * Column)) = Color; 
        }
    }
    */

   // TODO: Implement a switching mechanism.
}


// Plot a single pixel at the specified location with the specified color.
void DrawPixel(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU, uint32_t x, uint32_t y, uint32_t Color) {
    // We can't draw off the screen, so fill the screen in that case.
    // TODO: implement the border coloring here
    if(y > GPU.Info->VerticalResolution || x > GPU.Info->HorizontalResolution) {
        FillScreen(GPU, Color);
    } else {
        // If the pixel is in bounds:
        // Find the offset from the base of the framebuffer by:
        // y * width + x
        // 4 bits per pixel, so multiply by 4 to get the start of the correct pixel
        // Set it to the color we want.
        *(uint32_t* )(GPU.FrameBufferBase + (y * GPU.Info->PixelsPerScanline + x) * 4) = Color;
    }
}

// TODO: Rectangle, Triangle drawing.

void WriteChar(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU, int Char, uint32_t Height, uint32_t Width, uint32_t FontColor, uint32_t HighlightColor) {
    if(Height > GPU.Info->VerticalResolution || Width > GPU.Info->HorizontalResolution) {
        FillScreen(GPU, FontColor);
    } else {
        // This writes to the top left of the screen.
        // TODO: Automatically determine the next position using GPU.Info->cursorPos
        RenderText(GPU, Char, Height, Width, FontColor, HighlightColor, 0, 0, 1, 0);
    }
}

void WriteCharPos(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU, int Char, uint32_t Height, uint32_t Width, uint32_t FontColor, uint32_t HighlightColor, uint32_t x, uint32_t y) {
    if(Height > GPU.Info->VerticalResolution || Width > GPU.Info->HorizontalResolution) {
        FillScreen(GPU, 0x00FF0000);
    } else if(y > GPU.Info->VerticalResolution || x > GPU.Info->HorizontalResolution) {
        FillScreen(GPU, 0x0000FF00);
    } else if ((y + Height) > GPU.Info->VerticalResolution || (x + Width) > GPU.Info->HorizontalResolution) {
        FillScreen(GPU, 0x000000FF);
    }

    RenderText(GPU, Char, Height, Width, FontColor, HighlightColor, x, y, 1, 0);
}

void WriteScaledString(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU, const char* String, uint32_t Height, uint32_t Width, uint32_t FontColor, uint32_t HighlightColor, uint32_t x, uint32_t y, uint32_t Scale) {
    if(Height > GPU.Info->VerticalResolution || Width > GPU.Info->HorizontalResolution) {
        FillScreen(GPU, 0x00FF0000);
    } else if( y > GPU.Info->VerticalResolution || x > GPU.Info->HorizontalResolution) {
        FillScreen(GPU, 0x0000FF00);
    } else if ((y + Scale * Height) > GPU.Info->VerticalResolution || (x + Scale * Width) > GPU.Info->HorizontalResolution) {
        FillScreen(GPU, 0x000000FF);
    }

    uint32_t Char = 0;
    while(String[Char] != '\0') {
        RenderText(GPU, String[Char], Height, Width, FontColor, HighlightColor, x, y, Scale, Char);
        Char++;
    }
}

// Render characters to the screen.

void RenderText(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU, int Char, uint32_t Height, uint32_t Width, uint32_t FontColor, uint32_t HighlightColor, uint32_t x, uint32_t y, uint32_t Scale, uint32_t INDX) {
    uint32_t Y = Width >> 3, X = 0;

    if((Width & 0x7) != 0) {
        Y++;
    }

    for(uint32_t Row = 0; Row < Height; Row++) {
        for(uint32_t Bit = 0; Bit < Width; Bit++) {
            if ( ((Bit & 0x7) == 0) && (Bit > 0)) {
                X++;
            }

            if((DEFONT[Char][Row * Y + X] >> (Bit & 0x7)) & 1) {
                for(uint32_t ScaleY = 0; ScaleY < Scale; ScaleY++) {
                    for(uint32_t ScaleX = 0; ScaleX < Scale; ScaleX++) {
                        // This one is crazy. Stick with me.
                        *(uint32_t* )(GPU.FrameBufferBase + // Offset from the framebuffer, find the pixel..
                                ((y * GPU.Info->PixelsPerScanline + x) + // which is this many rows down, and this many pixels across,
                                Scale * (Row * GPU.Info->PixelsPerScanline + Bit) + // shift it around according to our scale..
                                (ScaleY * GPU.Info->PixelsPerScanline + ScaleX) + 
                                Scale * INDX * Width) * 4) // and find which column we need to be in. Multiply by 4 to navigate the 4bpp array,
                                = FontColor; // And set the color of the pixel.
                    }
                }
            } else {
                // We need to draw the pixel transparently..
                for(uint32_t ScaleY = 0; ScaleY < Scale; ScaleY++) {
                    for(uint32_t ScaleX = 0; ScaleX < Scale; ScaleX++) {
                        if(HighlightColor != 0xFF000000) {
                          *(UINT32*)(GPU.FrameBufferBase +
                                    ((y*GPU.Info->PixelsPerScanline + x) + 
                                    Scale * (Row * GPU.Info->PixelsPerScanline + Bit) + 
                                    (ScaleY * GPU.Info->PixelsPerScanline + ScaleX) + 
                                    Scale * INDX * Width)*4)
                                    = HighlightColor;

                        }
                    }
                }
            }
        }
    }
}

// TODO: Custom Bitmap functions