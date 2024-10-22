#pragma once
#include <string.h>
#include <GLFW/glfw3.h>

typedef struct {
    const char *name;
    int key;
} KeyMapping;

// Gets the GLFW keycode from the given key name string. Returns -1 if not found.
int getKeyFromString (const char *keyName);

