#pragma once
#include <lua.h>
#include "putils/defines.h"
#include "putils/pstring.h"

#define MYTE_CONFIG_NAMESPACE "Myte"

typedef struct {
    i32 a;
    f32 b;
    string c;
    bool d;
} Config;

Config *configCreate(void);
void configLoad(Config *config, lua_State *L);
void configDestroy(Config *config);
