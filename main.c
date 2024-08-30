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

void print_representation_linear () {
    char *buffer_content = buffer_string(gb);

    if (strlen(buffer_content) == 0 || cursor_pos == 0){
        printf("|");
    }
    
    for (size_t i = 0; i < strlen(buffer_content); i++) {
        if (buffer_content[i] == '\n') {
             printf("&");
        } else {
            printf("%c", buffer_content[i]);
        }
        if (i == cursor_pos - 1) {
                printf("|");
            }
    }

    printf("\n");
    if (buffer_content[cursor_pos] != '\n') {
            printf("Current char: %d\n", gb->buffer[cursor_pos]);
    } else {
        printf("Current char: &\n");
    }
    
    free(buffer_content);
}

void print_representation_pretty () {
    char *buffer_content = buffer_string(gb);

    if (strlen(buffer_content) == 0 || cursor_pos == 0){
        printf("|");
    }
    
    for (size_t i = 0; i < strlen(buffer_content); i++) {
        
        printf("%c", buffer_content[i]);
        if (i == cursor_pos - 1) {
            printf("|");
        }
    }

    printf("\n");
    free(buffer_content);
}

void increment_cursor_pos () {
    if (cursor_pos < (gb->buffer_size - (gb->gap_end - gb->gap_start))) {
        cursor_pos ++;
    }
}

void decrement_cursor_pos () {
    if (cursor_pos > 0) {
        cursor_pos --;
    }
}

// Keep track of lines in a variable in the gapbuffer. The dude who's writing a text editor in Odin did it.

void move_cursor_up(GapBuffer *gb) {
    
    if (cursor_pos == 0) {
        return;
    }

    // Move to start of line
    size_t new_cursor_pos = cursor_pos
    while()
}

void move_cursor_down (GapBuffer *gb) {
    // Find the start of the current line
    size_t start_of_line = cursor_pos;
    while(gb->buffer[start_of_line - 1] != '\n' && start_of_line != 0) {
        start_of_line --;
    }

    // Calculate the current column we are on.
    size_t cols_current_line = cursor_pos - start_of_line;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE){
        decrement_cursor_pos();
        print_representation_linear();
    } else if (key == GLFW_KEY_UP && action == GLFW_RELEASE){
        move_cursor_up(gb);
        print_representation_linear();
    } else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE){
        move_cursor_down(gb);
        print_representation_linear();
    } else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE){
        increment_cursor_pos();
        print_representation_linear();
    } else if (key == GLFW_KEY_BACKSPACE && action == GLFW_RELEASE){
        remove_char_before_buffer (gb, cursor_pos);
        decrement_cursor_pos();
        print_representation_linear();
    } else if (key == GLFW_KEY_DELETE && action == GLFW_RELEASE){
        remove_char_after_buffer (gb, cursor_pos);
        print_representation_linear();
    } else if (key == GLFW_KEY_ENTER && action == GLFW_RELEASE){
        insert_char(gb, cursor_pos, '\n');
        increment_cursor_pos();
        print_representation_linear();
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;

    insert_char(gb, cursor_pos, (char)codepoint);
    increment_cursor_pos();
 
    print_representation_linear();
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
		i8 *loremIpsum = buffer_string(gb);
        rect quad = rect_init(10, 10, 100, 100);
		Color color = COLOR_MAGENTA;
		Color color_font = COLOR_ORANGE;
		vec2 text_pos = vec2_init(10, 500);
        
		// Render stuff goes here
		Render_Push_Quad_C(&renderer, quad, color);
		Render_Push_Char(&renderer, font_id, loremIpsum, &text_pos, color_font);

        Render_End_Frame(&renderer);
        glfwSwapBuffers(window);
    }
    Render_Free(&renderer);

    glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
} 