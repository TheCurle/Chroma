#include <windows/wds-core.h>
#include <kernel/chroma.h>

uint8_t rand() {
    static uint16_t seed = 14714;
    return (uint8_t) (seed = (12657 * seed + 12345) % 256);
}

void Window::paint() {
    uint32_t color = 0xFF000000 | rand() << 16 | rand() << 8 | rand();

    DrawFilledRect(frame, x, y, width, height, color);
}

void windowing_init() {
    Frame frame = { (uint32_t*) &fb, bootldr.fb_width, bootldr.fb_height };

    auto* win1 = new Window(10, 10, 300, 200, &frame);
    auto* win2 = new Window(100, 150, 400, 400, &frame);
    auto* win3 = new Window(200, 100, 200, 600, &frame);

    win1->paint();
    win2->paint();
    win3->paint();
}