#include "gui.h"

#if defined(CLI_ONLY)
int gui_run(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fprintf(stderr, "GUI is not supported in this CLI-only build.\n");
    return 1;
}
#elif defined(USE_GTK)
extern int gui_gtk_main(int argc, char **argv);
int gui_run(int argc, char **argv) {
    return gui_gtk_main(argc, argv);
}
#elif defined(_WIN32)
#include <windows.h>
extern int gui_win32_main(HINSTANCE hInstance, int nCmdShow);
int gui_run(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return gui_win32_main(GetModuleHandleA(NULL), SW_SHOWDEFAULT);
}
#else
int gui_run(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fprintf(stderr, "GUI support is not enabled in this build.\n");
    fprintf(stderr, "Please build with GTK 3 (USE_GTK=1) or run on Windows.\n");
    return 1;
}
#endif
