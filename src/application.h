#pragma once
//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "renderer.h"
#include "font.h"
#include "editor.h"
#include "config.h"
#include "keys.h"
#include "context.h"

#define MAX_COMMANDS 100

typedef struct {
    int key;
    int mods;
    void (*command)();
    CommandType type;
} KeyBind;

typedef struct {
    char *name;
    void (*command)();
} Command;

typedef struct {
    // The window for this application - this lives for the entire program.
    GLFWwindow *window;

    // The renderer for this application - this lives for the entire program.
    Renderer renderer;
    
    Editor editor;
    Config config;
    ColorTheme theme;

    char *status_message;
    f32 status_disp_time;

    bool mouse_held;

    Command commands[MAX_COMMANDS];
    KeyBind keybinds[MAX_COMMANDS];
    size_t numCommands;
    size_t numKeybinds;

    AppContext ctx;
} Application;

void applicationInit(Application *app, int argc, char **argv);
void applicationDestroy(Application *app);
void applicationReload(Application *app);
void applicationSetStatusMessage(Application *app, const char *msg, f32 t);

void applicationUpdate(Application *app, f64 delta_time);
void applicationRender(Application *app, f64 delta_time);

void applicationProcessEditorInput (Application *app, int key, int scancode, int action , int mods);

void applicationRegisterCommand(Application *app, char *name, void (*command)());
void applicationBindKey(Application *app, int key, int mods, const char *commandName, CommandType type);

// TODO come up with naming convention for commands
void Command_moveRight(Application *app);
void Command_moveForwardWord(Application *app);
void Command_selectRight(Application *app);
void Command_selectForwardWord(Application *app);

void Command_moveLeft(Application *app);
void Command_moveBackwardWord(Application *app);
void Command_selectLeft(Application *app);
void Command_selectBackwardWord(Application *app);

void Command_moveUp(Application *app);
void Command_selectUp(Application *app);

void Command_moveDown(Application *app);
void Command_selectDown(Application *app);

void Command_deleteLeft(Application *app);
void Command_deleteWordLeft(Application *app);

void Command_deleteRight(Application *app);
void Command_deleteWordRight(Application *app);

void Command_unselect(Application *app);

void Command_returnToEditor(Application *app);

void Command_openBrowser(Application *app);
void Command_write(Application *app);

void Command_decrementSelection(Application *app);
void Command_incrementSelection(Application *app);
void Command_openSelection(Application *app);

void Command_reloadConfig(Application *app);

void Command_openSaveDialog(Application *app);
void Command_submitSaveDialog(Application *app);

void Command_openNewFile(Application *app);