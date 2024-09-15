//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "font.h"
#include "editor.h"
#include "config.h"

typedef struct {
    // The window for this application - this lives for the entire program.
    GLFWwindow *window;

    // The renderer for this application - this lives for the entire program.
    Renderer renderer;
    
    Editor editor;
    Config config;
    ColorTheme theme;
    u32 font_id;
} Application;

void applicationInit(Application *app, int argc, char **argv);
void applicationDestroy(Application *app);
void applicationReload(Application *app);

void applicationUpdate(Application *app, f64 delta_time);
void applicationRender(Application *app, f64 delta_time);