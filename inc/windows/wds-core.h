#pragma once
#include <kernel/system/memory.h>
#include <kernel/video/draw.h>
#include <lainlib/list/list2.h>

// A single rectangular window.
struct Window {
    uint16_t x, y;          // Position
    uint16_t width, height; // Size
    Frame* frame;

    void paint();

    Window(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Frame* frame) :
     x(x), y(y), width(width), height(height), frame(frame) {}
};

// The struct that holds and manages all the accessible windows.
struct Desktop {
    explicit Desktop(Frame* frame) : frame(frame) {
        if (!(windows = new List())) {
            delete this;
        }
    }

    Window* createWindow(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        Window* wdw;
        if (!(wdw = new Window(x, y, width, height, frame)))
            return wdw;

        if (!windows->add((void*)wdw)) {
            delete wdw;
            return nullptr;
        }

        return wdw;
    }

    void paint() {
        uint32_t i;
        Window* wdw;

        DrawFilledRect(frame, 0, 0, frame->width, frame->height, 0xFFFF9933);

        for (i = 0; (wdw = (Window*)windows->get(i)); i++)
            wdw->paint();
    }

    List* windows;
    Frame* frame;
};

void windowing_init();
