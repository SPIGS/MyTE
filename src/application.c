#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include "buffer.h"
#include "config.h"
#include "putils/defines.h"
#include "putils/log.h"
#include "putils/pstring.h"
#include "application.h"
#include "command.h"

#define INITIAL_SCREEN_WIDTH 1280
#define INITIAL_SCREEN_HEIGHT 720

// NOTE: temporary
size_t cursor = 0;
void testCommand(Application *app) {
    LOG_DEBUG("Builtin command!", "");
    cursor = insertIntoBuf(app->buf, cursor, "ぁあぃいぅうぇえぉお");
    outputBufferString(app->buf, cursor);
}

void moveCursorLeft(Application *app) {
    cursor = getPrevGraphemeCursor(app->buf, cursor);
    outputBufferString(app->buf, cursor);
}

void moveCursorRight(Application *app) {
    cursor = getNextGraphemeCursor(app->buf, cursor);
    outputBufferString(app->buf, cursor);
}

void deleteGraphemeLeft(Application *app) {
    removeGraphemeBeforeGap(app->buf, cursor);
    cursor = getPrevGraphemeCursor(app->buf, cursor);
    outputBufferString(app->buf, cursor);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    UNUSED(scancode);
    UNUSED(mods);

    Application *app = (Application *)glfwGetWindowUserPointer(window);

    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        for (size_t i = 0; i < app->reg->num_binds; i++) {
            if (app->reg->binds[i].key == key && app->reg->binds[i].mods == mods) {
                registryExecuteCommand(app->reg, app, app->reg->binds[i].cmd_name);
            }
        }
    }
}

// NOTE: temporary
void characterCallback(GLFWwindow *window, unsigned int codepoint) {
    LOG_DEBUG("%c", codepoint);
    Application *app = glfwGetWindowUserPointer(window);
    char buffer[2] = {codepoint, '\0'};
    cursor = insertIntoBuf(app->buf, cursor, buffer);
    outputBufferString(app->buf, cursor);
}

Application *applicationNew(int argc, char **argv) {
    if (!glfwInit()) {
        LOG_ERROR("Failed to init GLFW", "");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    Application *app = (Application *)malloc(sizeof(Application));
    app->window = glfwCreateWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "Myte", NULL, NULL);
    if(!app->window) {
        glfwTerminate();
        LOG_ERROR("Failed to initialize GLFW window.", "");
        return NULL;
    }

    glfwMakeContextCurrent(app->window);
    glfwSetWindowUserPointer(app->window, app);
    glfwSetKeyCallback(app->window, keyCallback);
    glfwSetCharCallback(app->window, characterCallback);
    
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        LOG_ERROR("Failed to initialize GLEW.", "");
        return NULL;
    }

    LOG_INFO("OpenGL ver. %s", glGetString(GL_VERSION));

    // Load default config
    app->conf = configCreate();

    // Register commands
    app->reg = registryNew();
    registryPushCommand(app->reg,  "testBuiltin", CMD_TYPE_BUILTIN, testCommand);
    registryPushCommand(app->reg, "moveCursorLeft", CMD_TYPE_BUILTIN, moveCursorLeft);
    registryPushCommand(app->reg, "moveCursorRight", CMD_TYPE_BUILTIN, moveCursorRight);
    registryPushCommand(app->reg, "deleteGraphemeLeft",CMD_TYPE_BUILTIN, deleteGraphemeLeft);

    // Setup lua context
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    apiSetupLuaEnv(L, app->reg);
    // Run the init.lua file
    if (luaL_dofile(L, "init.lua") != LUA_OK) {
        LOG_WARN("Lua Error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    app->L = L;
    configLoad(app->conf, L);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    app->buf = gapBufferNew(64);

    return app;
}

void applicationDestroy(Application *app) {
    glfwDestroyWindow(app->window);
    lua_close(app->L);
    configDestroy(app->conf);
    gapBufferDestroy(app->buf);
    glfwTerminate();
}

void applicationUpdate(Application *app, f64 delta_time) {
    glfwPollEvents();

    // Read the command queue
    if (stringLength(app->reg->lua_cmd_queue) > 0) {
        LOG_DEBUG("APplicatin read command %s from queue!", app->reg->lua_cmd_queue);
        registryExecuteCommand(app->reg, app, app->reg->lua_cmd_queue);
        stringClear(app->reg->lua_cmd_queue);
    }
}
void applicationRender(Application *app, f64 delta_time) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.5, 1.0);
    glfwSwapBuffers(app->window);
}
