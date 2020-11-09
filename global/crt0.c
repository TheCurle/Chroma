
extern void Exit(int code);
extern int Main();

extern void _init();

void _start() {
    _init();
    unsigned long long zero = 0;
    __asm__ __volatile__("movq %%rbp, %[input]" : : [input] "m" (zero) : );
    int ex = Main();
    Exit(ex);
}