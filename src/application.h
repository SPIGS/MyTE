//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "font.h"
#include "editor.h"
#include "config.h"
#include "keys.h"

#define MAX_COMMANDS 100

typedef struct {
    // The window for this application - this lives for the entire program.
    GLFWwindow *window;

    // The renderer for this application - this lives for the entire program.
    Renderer renderer;
    
    Editor editor;
    Config config;
    ColorTheme theme;
    u32 font_id;

    char *status_message;
    f32 status_disp_time;

    bool mouse_held;

    Command commands[MAX_COMMANDS];
    KeyBind keybinds[MAX_COMMANDS];
    size_t numCommands;
    size_t numKeybinds;
} Application;

void applicationInit(Application *app, int argc, char **argv);
void applicationDestroy(Application *app);
void applicationReload(Application *app);
void applicationSetStatusMessage(Application *app, const char *msg, f32 t);

void applicationUpdate(Application *app, f64 delta_time);
void applicationRender(Application *app, f64 delta_time);

void applicationProcessEditorInput (Application *app, int key, int scancode, int action , int mods);
void applicationProcessBrowserInput (Application *app, int key, int scancode, int action , int mods);

void applicationRegisterCommand(Application *app, char *name, void (*command)());
void applicationBindKey(Application *app, int key, int mods, const char *commandName);
void Command_Test(Application *app);
void Command_moveRight(Application *app);