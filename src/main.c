#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "application.h"

int main (int argc, char **argv) {
    Application app;
    applicationInit(&app, argc, argv);

    f64 last_frame_time = 0.0f;
    while (!glfwWindowShouldClose(app.window)) {
        f64 cur_fame_time = (f64)glfwGetTime();
        f64 delta_time = cur_fame_time - last_frame_time;
        last_frame_time = cur_fame_time;

        applicationUpdate(&app, delta_time);
        applicationRender(&app, delta_time);
    }
    applicationDestroy(&app);
    return 0;
}