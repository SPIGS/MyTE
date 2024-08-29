#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "font.h"


void resize_window(GLFWwindow *window, int width, int height) {
	Renderer *r = glfwGetWindowUserPointer(window);
	Render_Resize_Window(r, width, height);
}

int main () {

    // Initialize glfw
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	GLFWwindow* window = glfwCreateWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "Hello World", NULL, NULL);
	if (!window) {
		glfwTerminate();
		fprintf(stderr, "Failed to initialize GLFW window.\n");
		return 1;
	}
    glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to init glew\n");
		return 1;
	}

	printf("OpenGL ver. %s\n", glGetString(GL_VERSION));

    Renderer renderer;
    Render_Init(&renderer, color_from_hex(0x007777FF));
	u32 font_id = Render_Load_Font(&renderer, "fonts/iosevka-firamono.ttf", 64);

	glfwSetWindowUserPointer(window, &renderer);
	glfwSetFramebufferSizeCallback(window, resize_window);	

	i8 *text = "Hello World, Goodbye World!";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Render_Begin_Frame(&renderer);

        rect quad = rect_init(10, 10, 100, 100);
		Color color = COLOR_MAGENTA;
		Color color_font = COLOR_ORANGE;
		vec2 text_pos = vec2_init(100, 500);
        
		// Render stuff goes here
		Render_Push_Quad_C(&renderer, quad, color);
		Render_Push_Char(&renderer, font_id, text, &text_pos, color_font);

        Render_End_Frame(&renderer);
        glfwSwapBuffers(window);
    }
    Render_Free(&renderer);

    glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
} 
