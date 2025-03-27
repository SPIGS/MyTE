#include "color.h"

Color initColor(f32 r, f32 g, f32 b, f32 a) {
  return (Color) { r, g, b , a};
}

Color colorFromHex(u32 hex) {
    Color result;
    u32 r = (hex >> (3 * 8)) & 0xFF;
    u32 g = (hex >> (2 * 8)) & 0xFF;
    u32 b = (hex >> (1 * 8)) & 0xFF;
    u32 a = (hex >> (0 * 8)) & 0xFF;
    result.r = r / 255.0f;
    result.g = g / 255.0f;
    result.b = b / 255.0f;
    result.a = a / 255.0f;
    return result;
}
