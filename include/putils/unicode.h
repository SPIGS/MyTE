#pragma once
#include <stdint.h>
#include <stddef.h>

typedef uint32_t UnicodeChar;

UnicodeChar packUTF8(const char *bytes, size_t len);
char *unpackUTF8(UnicodeChar packed);
size_t getUTF8Size(UnicodeChar uc);

