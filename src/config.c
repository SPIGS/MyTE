#include "config.h"
#include "putils/log.h"
#include "putils/pstring.h"
#include <lua.h>


static int getFieldInt(lua_State* L, const char *name, int default_val) {
    int val = default_val;
    lua_pushstring(L, name);
    lua_gettable(L, -2); // get table[name]
    if (!lua_isnumber(L, -1)) {
        LOG_WARN("Lua Error: Couldn't read variable number (int) \'%s\' - using default value.", name);
    } else {
        val = (int)lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
    return val;
}

static double getFieldDouble(lua_State* L, const char *name, double default_val) {
    double val = default_val;
    lua_pushstring(L, name);
    lua_gettable(L, -2); // get table[name]
    if (!lua_isnumber(L, -1)) {
        LOG_WARN("Lua Error: Couldn't read variable number (double) \'%s\' - using default value.", name);
    } else {
        val = (double)lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
    return val;
}

static string getFieldString(lua_State* L, const char *name, char *default_val) {
    string val = stringNew("");
    lua_pushstring(L, name);
    lua_gettable(L, -2); // get table[name]
    if (!lua_isstring(L, -1)) {
        LOG_WARN("Lua Error: Couldn't read variable string \'%s\' - using default value.", name);
    } else {
        const char* str = lua_tostring(L, -1);
        if (str) {
            val = stringCatStr(val, str);
        } else {
            val = stringCatStr(val, default_val);
        }
    }
    lua_pop(L, 1);
    return val;
}

static bool getFieldBool(lua_State* L, const char *name, bool default_val) {
    bool val = default_val;
    lua_pushstring(L, name);
    lua_gettable(L, -2); // get table[name]
    if (!lua_isboolean(L, -1)) {
        LOG_WARN("Lua Error: Couldn't read variable boolean \'%s\' - using default value.", name);
    } else {
        val = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
    return val;
}


Config *configCreate(void) {
    Config *conf = (Config *)malloc(sizeof(Config));
    *conf = (Config) {
        420,
        69.69f,
        stringNew("Hello world from C!"),
        false,
    };

    return conf;
}

void configLoad(Config *config, lua_State *L) {
    lua_getglobal(L, MYTE_CONFIG_NAMESPACE);
    if (!lua_istable(L, -1)) {
        LOG_WARN("Lua Error: Couldn't load configuration table - possibly missing or redefined to be something else; using defaults.", "");
    } else {
        config->a = getFieldInt(L, "a", 420);
        config->b = getFieldDouble(L, "b", 69.69f);
        config->c = getFieldString(L, "c", "Hello world from C!");
        config->d = getFieldBool(L, "d", false);
    }
    LOG_DEBUG("Loaded config.", "");
}

void configDestroy(Config *config) {
    if (config->c)
        stringFree(config->c);

    free(config);
}
