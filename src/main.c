#include <GLFW/glfw3.h>
#include <stdlib.h>
#include "application.h"
#include "putils/log.h"

int main (int argc, char **argv) {
    Application *app = applicationNew(argc, argv);
    if (!app) {
        LOG_ERROR("Failed to create new application.", "");
        exit(1);
    }

    f64 last_frame_time = 0.0f;
    while (!glfwWindowShouldClose(app->window)) {
        f64 cur_frame_time = (f64)glfwGetTime();
        f64 delta_time = cur_frame_time - last_frame_time;
        last_frame_time = cur_frame_time;

        applicationUpdate(app, delta_time);
        applicationRender(app, delta_time);
    }
    applicationDestroy(app);
    return 0;
}
