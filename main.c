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


GapBuffer *gb;
size_t cursor_pos = 0;

void move_cursor_up(GapBuffer *gb) {
    size_t column = get_column(gb, cursor_pos);
    size_t beginning_of_prev_line = get_beginning_of_prev_line_cursor(gb, cursor_pos);
    size_t prev_line_length = get_buffer_line_length(gb, beginning_of_prev_line);
    if (get_beginning_of_line_cursor(gb, cursor_pos) == 0) {
        prev_line_length = 0;
    }
    cursor_pos = beginning_of_prev_line + MIN(prev_line_length, column);
}

void move_cursor_down (GapBuffer *gb) {
    size_t column = get_column(gb, cursor_pos);
    size_t beginning_of_next_line = get_beginning_of_next_line_cursor(gb, cursor_pos);
    size_t next_line_length = get_end_of_line_cursor(gb, beginning_of_next_line) - beginning_of_next_line;
    cursor_pos = beginning_of_next_line + MIN(next_line_length, column);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action , int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        cursor_pos = get_prev_character_cursor(gb, cursor_pos);
           output_buffer_string(gb, get_cursor_idx(gb, cursor_pos));
    } else if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        move_cursor_up(gb);
           output_buffer_string(gb, get_cursor_idx(gb, cursor_pos));
    } else if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        move_cursor_down(gb);
           output_buffer_string(gb, get_cursor_idx(gb, cursor_pos));
    } else if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        cursor_pos = get_next_character_cursor(gb, cursor_pos);
           output_buffer_string(gb, get_cursor_idx(gb, cursor_pos));
    } else if (key == GLFW_KEY_BACKSPACE && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        remove_char_before_gap (gb, cursor_pos);
        cursor_pos = get_prev_character_cursor(gb, cursor_pos);
           output_buffer_string(gb, get_cursor_idx(gb, cursor_pos));
    } else if (key == GLFW_KEY_DELETE && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        remove_char_after_gap (gb, cursor_pos);
           output_buffer_string(gb, get_cursor_idx(gb, cursor_pos));
    } else if (key == GLFW_KEY_ENTER && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        insert_char(gb, cursor_pos, '\n');
        cursor_pos = get_next_character_cursor(gb, cursor_pos);
           output_buffer_string(gb, get_cursor_idx(gb, cursor_pos));
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    (void)window;

    insert_char(gb, cursor_pos, (char)codepoint);
    cursor_pos = get_next_character_cursor(gb, cursor_pos);
    output_buffer_string(gb, get_cursor_idx(gb, cursor_pos));
}

void resize_window(GLFWwindow *window, int width, int height) {
	Renderer *r = glfwGetWindowUserPointer(window);
	Render_Resize_Window(r, width, height);
}

int main () {
	gb = create_gap_buffer(INITIAL_SIZE);

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
	u32 font_id = Render_Load_Font(&renderer, "fonts/iosevka-firamono.ttf", 48);

	glfwSetWindowUserPointer(window, &renderer);
	glfwSetFramebufferSizeCallback(window, resize_window);
	glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Render_Begin_Frame(&renderer);
		char * buffer_string = get_buffer_string(gb);
        rect quad = rect_init(10, 10, 100, 100);
		Color color = COLOR_MAGENTA;
		Color color_font = COLOR_ORANGE;
		vec2 text_pos = vec2_init(10, 500);
        
		// Render stuff goes here
		Render_Push_Quad_C(&renderer, quad, color);
		//Render_Push_Char(&renderer, font_id, loremIpsum, &text_pos, color_font);
        Render_Push_Buffer(&renderer, font_id, buffer_string, cursor_pos, &text_pos, color_font);
        free(buffer_string);
        Render_End_Frame(&renderer);
        glfwSwapBuffers(window);
    }
    Render_Free(&renderer);
    destroy_gap_buffer(gb);

    glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
} 