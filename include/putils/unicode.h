#pragma once
#include "putils/defines.h"
#include "putils/pstring.h"

typedef uint32_t UnicodeChar;

typedef struct {
    size_t size;
    size_t cap;
    UnicodeChar *s;
} UTF8String;

UTF8String UTF8StringNew(void);
void UTF8StringDestroy(UTF8String *s);
void UTF8StringPushChar(UTF8String *s, UnicodeChar c);
size_t UTF8StringGetSubstringSizeBytes(UTF8String *s, size_t beg, size_t end);

UnicodeChar packUTF8(const char *bytes, size_t len);
char *unpackUTF8(UnicodeChar packed);
string unpackUTF8String(UnicodeChar *packed_str, size_t len);
size_t getUTF8Size(UnicodeChar uc);

bool isspaceUTF8(UnicodeChar uc);
bool ispunctUTF8(UnicodeChar uc);
bool isalnumUTF8(UnicodeChar uc);
