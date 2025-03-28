#pragma once
#include <GLFW/glfw3.h>
#include <lua.h>
#include "config.h"
#include "putils/defines.h"
#include "api.h"
#include "editor.h"
#include "renderer.h"

typedef struct _Application{
    GLFWwindow *window;
    Renderer *r;
    lua_State *L;
    Config *conf;
    CommandRegistry *reg;
    Editor *ed;
} Application;

Application *applicationNew(int argc, char **argv);
void applicationDestroy(Application *app);

void applicationUpdate(Application *app, f64 delta_time);
void applicationRender(Application *app, f64 delta_time);

