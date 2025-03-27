#include "putils/pstring.h"

#define MAX_COMMANDS 100

typedef enum {
    CMD_TYPE_BUILTIN,
    CMD_TYPE_LUA
} CommandType;

typedef struct _Application Application;
typedef void (*CommandFunc)(Application *app);

typedef struct {
    string name;
    CommandType type;
    CommandFunc builtin_cmd;
} Command;

typedef struct {
    i32 key;
    i32 mods;
    string cmd_name;
} KeyBind;

typedef struct _CommandRegistry{
    Command cmds[MAX_COMMANDS];
    size_t num_cmds;

    KeyBind binds[MAX_COMMANDS];
    size_t num_binds;

    // a "list" of commands that have been called from lua
    string lua_cmd_queue;
} CommandRegistry;

CommandRegistry *registryNew(void);
void registryDestroy(CommandRegistry *reg);
void registryPushCommand(CommandRegistry *reg, const char *name, CommandType type, CommandFunc func);
void registryPushKeyBind(CommandRegistry *reg, i32 key, i32 mods, const char *cmd_name);
void registryExecuteCommand(CommandRegistry *reg, Application *app, const char *cmd_name);
