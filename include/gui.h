/**
 * @file gui.h
 * @brief Graphical user interface entry point and platform abstraction.
 *
 * Provides a unified entry point to launch the graphical desktop interface.
 * Dispatches to native Win32 GDI/ComCtl32 implementation on Windows,
 * or GTK3 on Linux/Unix platforms.
 */

#ifndef GUI_H
#define GUI_H

#include "compiler.h"

/**
 * @brief Launches the interactive desktop application window.
 *
 * @param argc Command-line argument count.
 * @param argv Command-line argument vector.
 * @return Exit status code returned to host OS.
 */
int gui_run(int argc, char **argv);

#endif /* GUI_H */
