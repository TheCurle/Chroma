#pragma once
#include <stdint.h>
/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

/**
 * Drawing routines for screen manipulation.
 * This will be pulled out into the Helix library soon.
 *
 * Set color with PrintInfo.
 */

void _swap_size_t(size_t a, size_t b);

int abs(int x);

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
    size_t      horizontalOffset;
    size_t      charsPerRow;
    size_t      rowsPerScrn;
    uint32_t    scrlMode;

    size_t      screenWidth;
    size_t      screenHeight;
} PRINTINFO;

extern PRINTINFO PrintInfo;

void DrawPixel(size_t x, size_t y);
void FillScreen();

void SetForegroundColor(uint32_t color);
uint32_t GetForegroundColor();
void SetBackgroundColor(uint32_t color);
uint32_t GetBackgroundColor();

void DrawLine(size_t x0, size_t y0, size_t x1, size_t y1);

void DrawFilledRect(size_t x, size_t y, size_t width, size_t height);
void DrawLineRect(size_t x, size_t y, size_t width, size_t height, size_t thickness);
void DrawFilledRoundedRect(size_t x, size_t y, size_t width, size_t height, size_t radius);
void DrawLineRoundedRect(size_t x, size_t y, size_t width, size_t height, size_t radius);

void DrawFilledCircle(size_t centerX, size_t centerY, size_t radius);
void DrawLineCircle(size_t centerX, size_t centerY, size_t radius);
void DrawLineCircleCorners(size_t centerX, size_t centerY, size_t radius, char cornerMask);