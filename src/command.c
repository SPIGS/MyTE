#include "command.h"
#include <string.h>
#include "putils/log.h"
#include "putils/pstring.h"
#include "application.h"

#define INITIAL_COMMAND_SIZE 64

CommandRegistry *registryNew(void) {
    CommandRegistry *reg = (CommandRegistry*)malloc(sizeof(CommandRegistry));
    if (!reg) return NULL;
    reg->num_binds = 0;
    reg->num_cmds = 0;
    reg->lua_cmd_queue = stringNew("");
    return reg;
}

void registryDestroy(CommandRegistry *reg) {
    for (size_t i = 0; i < reg->num_cmds; i++) {
        stringFree(reg->cmds[i].name);
    }

    for (size_t i = 0; i < reg->num_binds; i++) {
        stringFree(reg->binds[i].cmd_name);
    }
    stringFree(reg->lua_cmd_queue);
}

static bool registryContains(CommandRegistry *reg, const char *name) {
    if (reg->num_cmds == 0) return false;

    size_t low = 0;
    size_t high = reg->num_cmds;
    while (low < high) {
        size_t mid = (low + high) / 2;
        i32 c = strcmp(reg->cmds[mid].name, name);
        if (c == 0) {
            return true;
        }
        if (c < 0) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return false;
}

void registryPushCommand(CommandRegistry *reg, const char *name, CommandType type, CommandFunc func) {
    if (!registryContains(reg, name)) {
        reg->cmds[reg->num_cmds] = 
            (Command) {
                stringNew(name),
                type,
                func,
            };
        reg->num_cmds++;

        if (type == CMD_TYPE_BUILTIN) {
            LOG_INFO("Registered builtin command \'%s\'.", name);
        } else {
            LOG_INFO("Registered lua user command \'%s\'.", name);
        }
    } else {
        LOG_WARN("Cannot register function or command \'%s\', name already taken.", name);
    }
}

void registryPushKeyBind(CommandRegistry *reg, i32 key, i32 mods, const char *cmd_name) {
    reg->binds[reg->num_binds] = (KeyBind) {
        key,
        mods,
        stringNew(cmd_name)
    };
    reg->num_binds++;
}

void registryExecuteCommand(CommandRegistry *reg, Application *app, const char *cmd_name) {
    for (size_t i = 0; i < reg->num_cmds; i++) {
        if (stringEq(reg->cmds[i].name, cmd_name)) {
            switch (reg->cmds[i].type) {
                case CMD_TYPE_LUA:
                    LOG_DEBUG("Executing user-defined lua command!, %s", cmd_name);
                    callLuaFunction(app->L, cmd_name);
                    break;
                case CMD_TYPE_BUILTIN:
                    LOG_DEBUG("Executing builtin command, %s", cmd_name);
                    reg->cmds[i].builtin_cmd(app);
                    break;
            }
        }
    }
}
