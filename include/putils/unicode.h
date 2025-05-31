#pragma once
#include "putils/defines.h"
#include "putils/pstring.h"

typedef uint32_t UnicodeChar;

UnicodeChar packUTF8(const char *bytes, size_t len);
char *unpackUTF8(UnicodeChar packed);
string unpackUTF8String(UnicodeChar *packed_str, size_t len);
size_t getUTF8Size(UnicodeChar uc);

bool isspaceUTF8(UnicodeChar uc);
bool ispunctUTF8(UnicodeChar uc);
bool isalnumUTF8(UnicodeChar uc);
