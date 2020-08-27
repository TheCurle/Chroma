
extern void Exit(int code);
extern int Main();

extern void _init();

void _start() {
    _init();
    int ex = Main();
    Exit(ex);
}