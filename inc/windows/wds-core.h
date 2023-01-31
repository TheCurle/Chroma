#pragma once
#include <kernel/system/memory.h>
#include <kernel/video/draw.h>

struct Window {
    uint16_t x, y;          // Position
    uint16_t width, height; // Size
    Frame* frame;

    void paint();

    Window(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Frame* frame) :
     x(x), y(y), width(width), height(height), frame(frame) {}
};

void windowing_init();
