#pragma once

typedef struct {
    const char *name;
    int key;
} KeyNameMapping;

// Gets the GLFW keycode from the given key name string, Returns -1 if not found;
int getKeyFromString(const char *key_name);

