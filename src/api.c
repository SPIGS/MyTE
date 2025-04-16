#include "api.h"
#include "putils/log.h"
#include "putils/pstring.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <ctype.h>
#include <GLFW/glfw3.h>
#include "keys.h"
#include "command.h"

#define MYTE_CONFIG_NAMESPACE "Myte"

static CommandRegistry *REGISTRY = NULL;

// LUA API function: register command
int lua_registerCommand(lua_State* L) {
    const char *name = luaL_checkstring(L, 1);
    if (!name) {
        LOG_WARN("Unable to register lua user command \'%s\' - didn't receive a string.", name);
        return 0;
    }

    registryPushCommand(REGISTRY, name, CMD_TYPE_LUA, NULL);
    return 0;
}

// LUA API function: bind a key to a comman or function
int lua_bindKey(lua_State *L) {
    const char *keystroke = luaL_checkstring(L, 1);
    const char *command = luaL_checkstring(L, 2);

    // Parse key string
    int mod_bitmask = 0;
    int key_code = 0;
    string buffer = stringNew("");
    for (size_t i = 0; i < strlen(keystroke); i++) {
        char lc = tolower(keystroke[i]);
        if (isspace(lc)) {
            continue;
        } else if (lc == '+') {
            // determine modifers
            if (stringEq(buffer, "control") || stringEq(buffer, "ctrl")) {
                mod_bitmask = mod_bitmask | GLFW_MOD_CONTROL;
                stringClear(buffer);
            } else if (stringEq(buffer, "shift")) {
                mod_bitmask = mod_bitmask | GLFW_MOD_SHIFT;
                stringClear(buffer);
            } else if (stringEq(buffer, "alt")) {
                mod_bitmask = mod_bitmask | GLFW_MOD_ALT;
                stringClear(buffer);
            } else if (stringEq(buffer, "buffer") || stringEq(buffer, "cmd") || stringEq(buffer, "windows")) {
                mod_bitmask = mod_bitmask | GLFW_MOD_SUPER;
                stringClear(buffer);
            } else {
                // We are looking at the name of a key.
                key_code = getKeyFromString(buffer);
                stringClear(buffer);
            }
        } else {
            buffer = stringCatChar(buffer, lc);
        }
    }
    key_code = getKeyFromString(buffer);
    registryPushKeyBind(REGISTRY, key_code, mod_bitmask, command);
    stringFree(buffer);
    LOG_INFO("Binding \'%s\' to \'%s\'.", command, keystroke);
    return 0;
}

// LUA API: execute a command from lua
int lua_executeCommand(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    if (!name) {
        LOG_WARN("Lua Error: expected a command name (string) got other.", "");
        return 0;
    }
    REGISTRY->lua_cmd_queue = stringCatStr(REGISTRY->lua_cmd_queue, name);
    return 0;
}

void callLuaFunction(lua_State *L, const char *name) {
    lua_getglobal(L, name);

    if (!lua_isfunction(L, -1)) {
        LOG_WARN("Attempt at calling non-function \'%s\' as function", name);
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 0, 0, 0) != 0) {
        LOG_WARN("Lua Error: Calling function \'%s\'", name);
        lua_pop(L, 1);
    }
}

void apiSetupLuaEnv(lua_State *L, CommandRegistry *reg) {
    REGISTRY = reg;
    lua_newtable(L);
    
    lua_pushcfunction(L, lua_registerCommand);
    lua_setfield(L, -2, "register");

    lua_pushcfunction(L, lua_bindKey);
    lua_setfield(L, -2, "bind");

    lua_pushcfunction(L, lua_executeCommand);
    lua_setfield(L, -2, "executeCommand");

    lua_setglobal(L, MYTE_CONFIG_NAMESPACE);
}
