#include "unicode.h"
#include <stdlib.h>

UnicodeChar packUTF8(const char *bytes, size_t len) {
    UnicodeChar uc = 0;
    for (size_t i = 0; i < len; i++) {
        uc |= ((UnicodeChar)(uint8_t)bytes[i]) << (i * 8);
    }
    return uc;
}

size_t getUTF8Size(UnicodeChar uc) {
    for (size_t i = 0; i < 4; i++) {
        if (((char)(uint8_t)((uc >> (i * 8)) & 0xFF)) == 0x00)
            return i;
    }
    return 0;
}

char *unpackUTF8(UnicodeChar packed) {
    char *out = (char *)malloc(5);
    if (!out) return NULL;

    for (size_t i = 0; i < 4; i++) {
        out[i] = ((char)(uint8_t)((packed >> (i * 8)) & 0xFF));
    }
    out[4] = '\0';
    return out;
}
