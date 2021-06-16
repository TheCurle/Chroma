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
} PRINTINFO;

extern PRINTINFO PrintInfo;

void DrawPixel(size_t x, size_t y);
void FillScreen();

void SetForegroundColor(uint32_t color);
uint32_t GetForegroundColor();
void SetBackgroundColor(uint32_t color);
uint32_t GetBackgroundColor();

void DrawFilledRect(size_t x, size_t y, size_t width, size_t height);
void DrawLineRect(size_t x, size_t y, size_t width, size_t height, size_t thickness);

// When height == width, this draws a circle.
void DrawOval(size_t centerX, size_t centerY, size_t height, size_t width);