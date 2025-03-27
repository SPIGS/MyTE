#pragma once
#include "defines.h"

typedef struct { f32 r; f32 g; f32 b; f32 a; } Color;

Color initColor(f32 r, f32 g, f32 b, f32 a);
Color colorFromHex(u32 hex);

/* Common Colors */

#define COLOR_BLACK     colorFromHex(0x000000FF)
#define COLOR_WHITE     colorFromHex(0xFFFFFFFF)
#define COLOR_SILVER    colorFromHex(0x808080FF)
#define COLOR_GRAY      colorFromHex(0x232323FF)
#define COLOR_RED       colorFromHex(0xFF0000FF)
#define COLOR_ORANGE    colorFromHex(0xFFA500FF)
#define COLOR_GOLD      colorFromHex(0xFFD700FF)
#define COLOR_YELLOW    colorFromHex(0xFFFF00FF)
#define COLOR_GREEN     colorFromHex(0x00FF00FF)
#define COLOR_CYAN      colorFromHex(0x00FFFFFF)
#define COLOR_BLUE      colorFromHex(0x0000FFFF)
#define COLOR_NAVY      colorFromHex(0x000080FF)
#define COLOR_PURPLE    colorFromHex(0x800080FF)
#define COLOR_MAGENTA   colorFromHex(0xFF00FFFF)

