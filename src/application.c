#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include "config.h"
#include "editor.h"
#include "putils/color.h"
#include "putils/defines.h"
#include "putils/log.h"
#include "putils/pmath.h"
#include "putils/pstring.h"
#include "application.h"
#include "command.h"
#include "renderer.h"



// The command name must be prefixed with "_" or the macro won't work
#define REGISTER_COMMAND(reg, command) \
    registryPushCommand(reg, #command, CMD_TYPE_BUILTIN, _##command);

// The command name must be prefixed with "_" or the macro won't work
#define COMMAND(command) \
    void _##command(Application *app)


COMMAND(splat) {
    LOG_DEBUG("Builtin command!", "");
    //editorInsert(app->ed, "ぁ");
    //editorInsert(app->ed, "ね");
    //editorInsert(app->ed, "À");
    editorInsert(app->ed, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\n€ƒ„…†‡ˆ‰Š‹ŒŽ˜™š›œžŸ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ\n·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ\nΓΔΛαβγδηθικλμνξπτυφχψ\nЖЗКНРУЭЯавжзклмнруфчьыэя\nᚠᚡᚢᚣᚤᚥᚦᚧᚨᚩᚪᚫᚬᚭᚮᚯᚰᚱᚲᚳᚴᚵᚶᚷᚸᚹᚺᚻᚼᚽᚾᚿᛀᛁᛂᛃᛄᛅᛆᛇᛈᛉᛊᛋᛌᛍᛎᛏᛐᛑᛒᛓᛔᛕᛖᛗᛘᛙᛚᛛᛜᛝᛞᛟᛠᛡᛢᛣᛤᛥᛦᛧᛨᛩᛪ᛫᛬᛭ᛮᛯᛰ\nԱԲԳԴԵԶԷԸԹԺԻԼԽԾԿՀՁՂՃՄՅՆՇՈՉՊՋՌՍՎՏՐՑՒՓՔՕՖՙ՚՛՜՝՞՟ՠաբգդեզէըթժիլխծկհձղճմյնշոչպջռսվտրցւփքօֆևֈ։֊\nぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんゔゕゖ゛゜ゝゞゟ\n");
}

COMMAND(moveCursorLeft) {
    editorMoveLeft(app->ed);
}

COMMAND(moveCursorRight) {
    editorMoveRight(app->ed);
}

COMMAND(deleteGraphemeLeft) {
    editorDeleteLeft(app->ed);
}

COMMAND(deleteGraphemeRight) {
    editorDeleteRight(app->ed);
}

COMMAND(moveCursorUp) {
    editorMoveUp(app->ed);
}

COMMAND(moveCursorDown) {
    editorMoveDown(app->ed);
}

COMMAND(moveCursorEndOfNextWord) {
    editorMoveEndOfNextWord(app->ed);
}

COMMAND(moveCursorBegOfPrevWord) {
    editorMoveBegOfPrevWord(app->ed);
}

COMMAND(deleteWordLeft) {
    editorDeleteWordLeft(app->ed);
}

COMMAND(deleteWordRight) {
    editorDeleteWordRight(app->ed);
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

        if (key == GLFW_KEY_ENTER) {
            char bytes[2] = {'\n', '\0'};
            editorInsert(app->ed, bytes);
        } else if (key == GLFW_KEY_TAB) {
            char bytes[2] = {'\t', '\0'};
            editorInsert(app->ed, bytes);
        }
    }
}

// NOTE: temporary
void characterCallback(GLFWwindow *window, unsigned int codepoint) {
    Application *app = glfwGetWindowUserPointer(window);
    char bytes[2] = {codepoint, '\0'};
    editorInsert(app->ed, bytes);
}

void resizeWindowCallback(GLFWwindow *window, int width, int height) {
    Application *app = glfwGetWindowUserPointer(window);
    rendererResizeWindow(app->r, width, height);
    // TODO: move this somewhere else
    
    app->ed->frame = rect(0, 0, (f32)width, (f32)height);
}

Application *applicationNew(int argc, char **argv) {
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

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
    glfwSetFramebufferSizeCallback(app->window, resizeWindowCallback);
    
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        LOG_ERROR("Failed to initialize GLEW.", "");
        return NULL;
    }

    LOG_INFO("OpenGL ver. %s", glGetString(GL_VERSION));

    // Load default config
    app->conf = configCreate();

    // Register built-in commands
    app->reg = registryNew();
    REGISTER_COMMAND(app->reg, splat);
    REGISTER_COMMAND(app->reg, moveCursorLeft);
    REGISTER_COMMAND(app->reg, moveCursorRight);
    REGISTER_COMMAND(app->reg, deleteGraphemeLeft);
    REGISTER_COMMAND(app->reg, deleteGraphemeRight);
    REGISTER_COMMAND(app->reg, moveCursorUp);
    REGISTER_COMMAND(app->reg, moveCursorDown);
    REGISTER_COMMAND(app->reg, moveCursorEndOfNextWord);
    REGISTER_COMMAND(app->reg, moveCursorBegOfPrevWord);
    REGISTER_COMMAND(app->reg, deleteWordLeft);
    REGISTER_COMMAND(app->reg, deleteWordRight);

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

    //Initialize renderer
    app->r = rendererNew(COLOR_BLACK);

    // Open an editor
    app->ed = editorNew(rect(0, 0, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT), app->r->line_height);

    return app;
}

void applicationDestroy(Application *app) {
    glfwDestroyWindow(app->window);
    rendererDestroy(app->r);
    lua_close(app->L);
    configDestroy(app->conf);
    editorDestroy(app->ed);
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
    editorUpdate(app->ed, delta_time);
}

void applicationRender(Application *app, f64 delta_time) {
    rendererBegin(app->r);

    renderEditor(app->r, app->ed, delta_time);

    float fps = 1.0f / delta_time;
    string fps_str = stringNew("");
    fps_str = stringFmt(fps_str, "FPS: %f", fps);
    stringFree(fps_str);

    f32 fps_x = 10.0;
    rendererText(app->r, fps_str, &fps_x, 10.0, COLOR_RED);
    renderQuad(app->r, 100.0, 100.0, 100.0, 100.0, COLOR_ORANGE);
    rendererEnd(app->r);
    glfwSwapBuffers(app->window);
}
