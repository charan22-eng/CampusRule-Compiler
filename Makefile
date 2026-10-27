# ==============================================================================
# Makefile for CampusRule Compiler (C11 Implementation)
# ==============================================================================

CC ?= gcc
ifeq ($(shell where $(CC) 2>NUL),)
    CC := C:/Users/resma/toolchain/w64devkit/bin/gcc.exe
endif

CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Iinclude -O2
DEBUG_FLAGS = -std=c11 -Wall -Wextra -Wpedantic -Iinclude -g -O0 -DDEBUG

BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
TEST_DIR = tests

ifeq ($(OS),Windows_NT)
    RM := C:/Users/resma/toolchain/w64devkit/bin/rm.exe -rf
    MKDIR := C:/Users/resma/toolchain/w64devkit/bin/mkdir.exe -p
else
    RM := rm -rf
    MKDIR := mkdir -p
endif

# ==============================================================================
# Core Compiler Sources
# ==============================================================================

CORE_SRCS = $(SRC_DIR)/token.c \
            $(SRC_DIR)/lexer.c \
            $(SRC_DIR)/ast.c \
            $(SRC_DIR)/parser.c \
            $(SRC_DIR)/semantic.c \
            $(SRC_DIR)/tac.c \
            $(SRC_DIR)/evaluator.c \
            $(SRC_DIR)/compiler.c

CORE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CORE_SRCS))

# ==============================================================================
# Test Sources
# ==============================================================================

TEST_SRCS = $(TEST_DIR)/test_lexer.c \
            $(TEST_DIR)/test_parser.c \
            $(TEST_DIR)/test_ast.c \
            $(TEST_DIR)/test_semantic.c \
            $(TEST_DIR)/test_tac.c \
            $(TEST_DIR)/test_evaluator.c \
            $(TEST_DIR)/test_main.c

TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.o,$(TEST_SRCS))

# ==============================================================================
# GUI Platform Configuration
# ==============================================================================

ifeq ($(USE_GTK),1)
    GUI_CFLAGS = $(shell pkg-config --cflags gtk+-3.0) -DUSE_GTK
    GUI_LIBS   = $(shell pkg-config --libs gtk+-3.0)
    GUI_SRCS   = $(SRC_DIR)/gui_gtk.c $(SRC_DIR)/gui.c
else
    ifeq ($(OS),Windows_NT)
        GUI_CFLAGS =
        GUI_LIBS   = -lcomctl32 -lgdi32
        GUI_SRCS   = $(SRC_DIR)/gui_win32.c $(SRC_DIR)/gui.c
    else
        GUI_CFLAGS = $(shell pkg-config --cflags gtk+-3.0) -DUSE_GTK
        GUI_LIBS   = $(shell pkg-config --libs gtk+-3.0)
        GUI_SRCS   = $(SRC_DIR)/gui_gtk.c $(SRC_DIR)/gui.c
    endif
endif

GUI_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(GUI_SRCS))

# ==============================================================================
# Executable Targets
# ==============================================================================

ifeq ($(OS),Windows_NT)
    TARGET_MAIN = $(BIN_DIR)/campusrule.exe
    TARGET_CLI  = $(BIN_DIR)/campusrule_cli.exe
    TARGET_TEST = $(BIN_DIR)/test_runner.exe
else
    TARGET_MAIN = $(BIN_DIR)/campusrule
    TARGET_CLI  = $(BIN_DIR)/campusrule_cli
    TARGET_TEST = $(BIN_DIR)/test_runner
endif

# IMPORTANT:
# Use forward slashes because w64devkit Make recipes run through a Unix-style
# shell. These paths work with both the Windows .exe targets and Unix targets.
RUN_TEST = ./$(TARGET_TEST)
RUN_MAIN = ./$(TARGET_MAIN)
RUN_CLI  = ./$(TARGET_CLI)

.PHONY: all cli gui test clean run run-cli debug help

# ==============================================================================
# Main Targets
# ==============================================================================

all: cli gui test

cli: $(TARGET_CLI)

gui: $(TARGET_MAIN)

# ==============================================================================
# Directory Creation
# ==============================================================================

$(OBJ_DIR):
	@$(MKDIR) $(OBJ_DIR)

$(BIN_DIR):
	@$(MKDIR) $(BIN_DIR)

# ==============================================================================
# Compilation Rules
# ==============================================================================

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(GUI_CFLAGS) -c $< -o $@

$(OBJ_DIR)/gui_cli.o: $(SRC_DIR)/gui.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -DCLI_ONLY -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ==============================================================================
# Build CLI Executable
# ==============================================================================

$(TARGET_CLI): $(CORE_OBJS) $(OBJ_DIR)/main.o $(OBJ_DIR)/gui_cli.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# ==============================================================================
# Build Desktop GUI Executable
# ==============================================================================

$(TARGET_MAIN): $(CORE_OBJS) $(GUI_OBJS) $(OBJ_DIR)/main.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(GUI_LIBS)

# ==============================================================================
# Build and Run Automated Tests
# ==============================================================================

$(TARGET_TEST): $(CORE_OBJS) $(TEST_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TARGET_TEST)
	@echo Running test suite...
	@$(RUN_TEST)

# ==============================================================================
# Run Targets
# ==============================================================================

run: $(TARGET_MAIN)
	@$(RUN_MAIN)

run-cli: $(TARGET_CLI)
	@$(RUN_CLI) --cli

# ==============================================================================
# Cleanup and Debug
# ==============================================================================

clean:
	@$(RM) $(OBJ_DIR) $(BIN_DIR)
	@echo Clean completed.

debug: CFLAGS = $(DEBUG_FLAGS)
debug: all

# ==============================================================================
# Help
# ==============================================================================

help:
	@echo CampusRule Compiler Build System
	@echo Available targets:
	@echo   make         - Build GUI, CLI, and run tests
	@echo   make cli     - Build CLI executable
	@echo   make gui     - Build desktop application
	@echo   make test    - Build and execute test suite
	@echo   make run     - Launch desktop GUI
	@echo   make run-cli - Execute CLI samples
	@echo   make clean   - Remove object and binary directories
	@echo   make debug   - Build with debug symbols