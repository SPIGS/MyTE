#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include "buffer.h"
#include "config.h"
#include "cursor.h"
#include "editor.h"
#include "modal.h"
#include "putils/color.h"
#include "putils/defines.h"
#include "putils/log.h"
#include "putils/pmath.h"
#include "putils/pstring.h"
#include "application.h"
#include "command.h"
#include "putils/unicode.h"
#include "renderer.h"
#include "context.h"


// the command name must be prefixed with "_" or the macro won't work
#define REGISTER_COMMAND(reg, command) \
    registryPushCommand(reg, #command, CMD_TYPE_BUILTIN, _##command);

// The command name must be prefixed with "_" or the macro won't work
#define COMMAND(command) \
    void _##command(Application *app)

static void closeModal(Application *app) {
    LOG_DEBUG("Destroying modal...", "");
    modalDestroy(app->modal);
    app->ed->focused = true;
    setCursorPosition(&app->ed->cursor, app->modal->cursor.screen_pos);
    app->ed->cursor.pos_anim_time = 0.0f;
    app->ed->cursor.moved_last_frame = true;
    app->modal = NULL;
}

COMMAND(splat) {
    LOG_DEBUG("Builtin command!", "");
    //editorInsert(app->ed, "ぁ");
    editorInsert(app->ed, "ね");
    //editorInsert(app->ed, "À");
    //editorInsert(app->ed, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\n€ƒ„…†‡ˆ‰Š‹ŒŽ˜™š›œžŸ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ\n·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ\nΓΔΛαβγδηθικλμνξπτυφχψ\nЖЗКНРУЭЯавжзклмнруфчьыэя\nᚠᚡᚢᚣᚤᚥᚦᚧᚨᚩᚪᚫᚬᚭᚮᚯᚰᚱᚲᚳᚴᚵᚶᚷᚸᚹᚺᚻᚼᚽᚾᚿᛀᛁᛂᛃᛄᛅᛆᛇᛈᛉᛊᛋᛌᛍᛎᛏᛐᛑᛒᛓᛔᛕᛖᛗᛘᛙᛚᛛᛜᛝᛞᛟᛠᛡᛢᛣᛤᛥᛦᛧᛨᛩᛪ᛫᛬᛭ᛮᛯᛰ\nԱԲԳԴԵԶԷԸԹԺԻԼԽԾԿՀՁՂՃՄՅՆՇՈՉՊՋՌՍՎՏՐՑՒՓՔՕՖՙ՚՛՜՝՞՟ՠաբգդեզէըթժիլխծկհձղճմյնշոչպջռսվտրցւփքօֆևֈ։֊\nぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんゔゕゖ゛゜ゝゞゟ\n");
}

COMMAND(moveCursorLeft) {
    if (app->modal == NULL)
        editorMoveLeft(app->ed);
}

COMMAND(moveCursorRight) {
    if (app->modal == NULL)
        editorMoveRight(app->ed);
}

COMMAND(deleteGraphemeLeft) {
    if (app->modal == NULL)
        editorDeleteLeft(app->ed);
}

COMMAND(deleteGraphemeRight) {
    if (app->modal == NULL)
        editorDeleteRight(app->ed);
}

COMMAND(moveCursorUp) {
    if (app->modal == NULL)
        editorMoveUp(app->ed);
}

COMMAND(moveCursorDown) {
    if (app->modal == NULL)
        editorMoveDown(app->ed);
}

COMMAND(moveCursorEndOfNextWord) {
    if (app->modal == NULL)
        editorMoveEndOfNextWord(app->ed);
}

COMMAND(moveCursorBegOfPrevWord) {
    if (app->modal == NULL)
        editorMoveBegOfPrevWord(app->ed);
}

COMMAND(deleteWordLeft) {
    if (app->modal == NULL)
        editorDeleteWordLeft(app->ed);
}

COMMAND(deleteWordRight) {
    if (app->modal == NULL)
        editorDeleteWordRight(app->ed);
}

COMMAND (showModal) {
    if (app->modal) {
        closeModal(app);
    } else {
        LOG_DEBUG("Showing modal...", "");
        app->modal = modalTextInit("The quick brown fox jumps over the lazy dog", app->ed->cursor.screen_pos);
        //app->modal = modalOptionInit("Hello world!");
        app->ed->focused = false;
    }
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

        if (app->modal == NULL) {
            if (key == GLFW_KEY_ENTER) {
                char bytes[2] = {'\n', '\0'};
                editorInsert(app->ed, bytes);
            } else if (key == GLFW_KEY_TAB) {
                char bytes[2] = {'\t', '\0'};
                editorInsert(app->ed, bytes);
            }
        } else {
            // Modal control keys
            // Just hardecode them for now
            switch (key) {
                case GLFW_KEY_LEFT:
                    if (mods & GLFW_MOD_CONTROL) {
                        modalMoveBegOfPrevWord(app->modal);
                    } else {
                        modalMoveCursorLeft(app->modal);
                    }
                break;
                case GLFW_KEY_RIGHT:
                    if (mods & GLFW_MOD_CONTROL) {
                        modalMoveCursorEndOfNextWord(app->modal);
                    } else {
                        modalMoveCursorRight(app->modal);
                    }
                break;
                case GLFW_KEY_UP:
                    modalMoveCursorBeginning(app->modal);
                break;
                case GLFW_KEY_DOWN:
                    modalMoveCursorEnd(app->modal);
                break;
                case GLFW_KEY_TAB:
                    modalCycleFocus(app->modal);
                break;
                case GLFW_KEY_ESCAPE:
                    closeModal(app);
                break;
                case GLFW_KEY_ENTER:
                    modalSubmit(app->modal);
                break;
                case GLFW_KEY_BACKSPACE:
                    if (mods & GLFW_MOD_CONTROL) {
                        modalDeleteWordLeft(app->modal);
                    } else {
                        modalDeleteLeft(app->modal);
                    }
                break;
                case GLFW_KEY_DELETE:
                    if (mods & GLFW_MOD_CONTROL) {
                        modalDeleteWordRight(app->modal);
                    } else {
                        modalDeleteRight(app->modal);
                    }
                break;
            }
        }
    }
}

// NOTE: temporary
void characterCallback(GLFWwindow *window, unsigned int codepoint) {
    Application *app = glfwGetWindowUserPointer(window);
    if (app->modal == NULL) {
        char bytes[2] = {codepoint, '\0'};
        editorInsert(app->ed, bytes);
    } else if (app->modal->type == MODAL_TYPE_TEXT){
        char bytes[2] = {codepoint, '\0'};
        modalInsert(app->modal, bytes);
    }
}

void resizeWindowCallback(GLFWwindow *window, int width, int height) {
    Application *app = glfwGetWindowUserPointer(window);
    rendererResizeWindow(app->r, width, height);
    // TODO: move this somewhere else
    
    app->ed->frame = rect(0, app->r->line_height, (f32)width, (f32)height - app->r->line_height);
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    Application *app = glfwGetWindowUserPointer(window);
    editorScrollWithMouseWheel(app->ed, yoffset);
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
    glfwSetScrollCallback(app->window, scrollCallback);
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
    REGISTER_COMMAND(app->reg, showModal);

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
    f32 status_line_height = app->r->line_height + 5.0;
    f32 h = INITIAL_SCREEN_HEIGHT - status_line_height;
    f32 y = status_line_height;
    app->ed = editorNew(rect(0, y, INITIAL_SCREEN_WIDTH, h), app->r->line_height);

    app->modal = NULL;

    return app;
}

void applicationDestroy(Application *app) {
    glfwDestroyWindow(app->window);
    rendererDestroy(app->r);
    lua_close(app->L);
    configDestroy(app->conf);
    editorDestroy(app->ed);

    if (app->modal) {
        modalDestroy(app->modal);
    }

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

    // Build the context
    AppContext ctx = {0};
    ctx.glyph_cache = app->r->glyphs;
    ctx.font_collection = app->r->font_collection;
    ctx.atlas = &app->r->atlas;
    ctx.screen_width = app->r->screen_width;
    ctx.screen_height = app->r->screen_height;
    ctx.glyph_width = app->r->glyph_width;

    if (app->modal) {
        if (app->modal->submitted) {
            if (app->modal->type == MODAL_TYPE_OPTION) {
                LOG_WARN("Submitted modal: %d", app->modal->selection);
            } else {
                LOG_WARN("Submitted modal:");
                outputBufferString(app->modal->buf, app->modal->cursor.buffer_idx);
                setCursorPosition(&app->ed->cursor, app->modal->cursor.screen_pos);
            }
            closeModal(app);
        }
    }

    editorUpdate(app->ed, &ctx, delta_time);
}

void applicationRender(Application *app, f64 delta_time) {
    rendererBegin(app->r);

    renderEditor(app->r, app->ed, delta_time);
    renderStatusLine(app->r, app->ed, delta_time);

    if (app->modal) {
        renderModal(app->r, app->modal, delta_time);
    }

    renderFPS(app->r, delta_time);

    rendererEnd(app->r);
    glfwSwapBuffers(app->window);
}
