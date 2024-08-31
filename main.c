#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "font.h"
#include "gapbuffer.h"

#define TAB_WIDTH 4

GapBuffer *gb;
size_t cursor_pos = 0;

void move_cursor_up(GapBuffer *gb) {
    size_t column = getBufColumn(gb, cursor_pos);
    size_t beginning_of_prev_line = getBeginningOfPrevLineCursor(gb, cursor_pos);
    size_t prev_line_length = getBufLineLength(gb, beginning_of_prev_line);
    if (getBeginningOfLineCursor(gb, cursor_pos) == 0) {
        prev_line_length = 0;
    }
    cursor_pos = beginning_of_prev_line + MIN(prev_line_length, column);
}

void move_cursor_down (GapBuffer *gb) {
    size_t column = getBufColumn(gb, cursor_pos);
    size_t beginning_of_next_line = getBeginningOfNextLineCursor(gb, cursor_pos);
    size_t next_line_length = getEndOfLineCursor(gb, beginning_of_next_line) - beginning_of_next_line;
    cursor_pos = beginning_of_next_line + MIN(next_line_length, column);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action , int mods)
{
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(mods);

    if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        cursor_pos = getPrevCharCursor(gb, cursor_pos);
    } else if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        move_cursor_up(gb);
    } else if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        move_cursor_down(gb);
    } else if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        cursor_pos = getNextCharCursor(gb, cursor_pos);
    } else if (key == GLFW_KEY_BACKSPACE && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        removeCharBeforeGap (gb, cursor_pos);
        cursor_pos = getPrevCharCursor(gb, cursor_pos);
    } else if (key == GLFW_KEY_DELETE && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        removeCharAfterGap (gb, cursor_pos);
    } else if (key == GLFW_KEY_ENTER && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        insertCharIntoBuf(gb, cursor_pos, '\n');
        cursor_pos = getNextCharCursor(gb, cursor_pos);
    } else if (key == GLFW_KEY_TAB && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        for (size_t i = 0; i< TAB_WIDTH; i++) {
            insertCharIntoBuf(gb, cursor_pos, ' ');
            cursor_pos = getNextCharCursor(gb, cursor_pos);
        }
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    UNUSED(window);
    insertCharIntoBuf(gb, cursor_pos, (char)codepoint);
    cursor_pos = getNextCharCursor(gb, cursor_pos);
}

void resize_window(GLFWwindow *window, int width, int height) {
	Renderer *r = glfwGetWindowUserPointer(window);
	rendererResizeWindow(r, width, height);
}

int main () {
	gb = gapBufferInit(INITIAL_SIZE);

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
    rendererInit(&renderer, COLOR_BLACK);
	u32 font_id = rendererLoadFont(&renderer, "fonts/iosevka-firamono.ttf", 48);

	glfwSetWindowUserPointer(window, &renderer);
	glfwSetFramebufferSizeCallback(window, resize_window);
	glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        rendererBegin(&renderer);
		char * buffer_string = getBufString(gb);
        rect quad = rect_init(10, 10, 100, 100);
		Color color = COLOR_GRAY;
		Color color_font = COLOR_WHITE;
		vec2 text_pos = vec2_init(10, 500);
        vec2 char_pos = vec2_init(10, 800);
        
		// Render stuff goes here
		renderQuad(&renderer, quad, color);
		renderChar(&renderer, font_id, '@', &char_pos, color_font);
        renderText(&renderer, font_id, buffer_string, cursor_pos, &text_pos, color_font);
        free(buffer_string);
        
        rendererEnd(&renderer);
        glfwSwapBuffers(window);
    }
    rendererDestroy(&renderer);
    gapBufferDestroy(gb);

    glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
} 