#include "unicode.h"
#include <stdlib.h>

UnicodeChar packUTF8(const char *bytes, size_t len_bytes) {
    UnicodeChar uc = 0;
    for (size_t i = 0; i < len_bytes; i++) {
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

bool isspaceUTF8(UnicodeChar uc) {
    return (uc == '\n') || (uc == ' ') || (uc == '\r') || (uc == '\t') || (uc == '\v') || (uc == '\f');
}

// Only handles punctuation characters found in ASCII right now
bool ispunctUTF8(UnicodeChar uc) {
    return (uc >= 33 && uc <= 47) || (uc >= 58 && uc <= 64) || (uc >= 91 && uc <= 96) || (uc >= 123 && uc <= 126);
}

// Only handles alphanumeric characters found in ASCII right now, everything outside of the ASCII
// range returns true
bool isalnumUTF8(UnicodeChar uc) {
    return (uc >=48 && uc <= 57) || (uc >= 65 && uc <= 70) || (uc >= 71 && uc <= 90) || (uc >= 97 && uc <= 120) || (uc >= 103 && uc <= 122) || ( uc >= 161);
}
