#pragma once
#include <string.h>
#include <GLFW/glfw3.h>
#include "util.h"

typedef struct {
    const char *name;
    int key;
} KeyMapping;

typedef struct {
    int key;
    int mods;
    void (*command)();
} KeyBind;

typedef struct {
    char *name;
    void (*command)();
} Command;

// Gets the GLFW keycode from the given key name string. Returns -1 if not found.
int getKeyFromString (const char *keyName);

