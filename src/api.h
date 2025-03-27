#pragma once
#include <lua.h>

typedef struct _CommandRegistry CommandRegistry;

void apiSetupLuaEnv(lua_State *L, CommandRegistry *reg);
void callLuaFunction(lua_State *L, const char *name);
