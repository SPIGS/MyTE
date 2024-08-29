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
    Color clr_color = {.r = 0.0f, .g = 0.4f, .b = 0.4f, .a = 0.0};
    Render_Init(&renderer, clr_color);

    u32 white_texture = Render_GetWhiteTexture();

	// PUtting the font stuff here for the time being - move this to the renderer

	// Load the library
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		printf("ERROR: Couldn't louad freetype library\n");
		exit(1);
	}

	// Load a face
	FT_Face face;
	if(FT_New_Face(ft, "iosevka-firamono.ttf", 0, &face)) {
		fprintf(stderr, "Could not open font\n");
		exit(1);
	}

	// Set the size of the font in pixels
	FT_Set_Pixel_Sizes(face, 0, 48);

	Free_Glyph_Atlas atlas;
	free_glyph_atlas_init(&atlas, face);

	glfwSetWindowUserPointer(window, &renderer);
	glfwSetFramebufferSizeCallback(window, resize_window);	

	u8 text[128];

	for (u8 i = 32; i < 128; i++) {
		text[i - 32] = i;
	}

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Render_Begin_Frame(&renderer);

        rect quad = rect_init(10, 10, 100, 100);
        Color color = { .r=1.0, .g=0.4, .b=1.0, .a=1.0};
		Color color_font = { .r=1.0, .g=1.0, .b=1.0, .a=1.0};
		vec2 text_pos = vec2_init(100, 500);
        // Render stuff goes here
        Render_Push_Quad_T(&renderer, quad, color, white_texture);
		Render_Push_Char(&renderer, &atlas, text, &text_pos, color_font);

        Render_End_Frame(&renderer);
        glfwSwapBuffers(window);
    }
    Render_Free(&renderer);

    glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
} 
