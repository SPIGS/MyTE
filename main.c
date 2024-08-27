#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gapbuffer.h"

//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


GapBuffer *gb;
size_t cursor_pos = 0;

void print_representation () {
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE){
        decrement_cursor_pos();
        print_representation();
        
    } else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE){
        increment_cursor_pos();
        print_representation();
    } else if (key == GLFW_KEY_BACKSPACE && action == GLFW_RELEASE){
        remove_char_before_buffer (gb, cursor_pos);
        decrement_cursor_pos();
        print_representation();
    } else if (key == GLFW_KEY_DELETE && action == GLFW_RELEASE){
        remove_char_after_buffer (gb, cursor_pos);
        print_representation();
    } else if (key == GLFW_KEY_ENTER && action == GLFW_RELEASE){
        insert_char(gb, cursor_pos, '\n');
        increment_cursor_pos();
        print_representation();
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;

    insert_char(gb, cursor_pos, (char)codepoint);
    increment_cursor_pos();
 
    print_representation();
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create window
	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window) {
		glfwTerminate();
		fprintf(stderr, "Failed to initialize GLFW window.\n");
		return 1;
	}
    glfwMakeContextCurrent(window);
	//glfwSetFramebufferSizeCallback(window, resizeWindow);

	if(glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to init glew\n");
		return 1;
	}

	printf("OpenGL ver. %s\n", glGetString(GL_VERSION));

    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);

    // Load font stuff

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
		glfwPollEvents();
    }

    glfwDestroyWindow(window);
	glfwTerminate();
    destroy_gap_buffer(gb);
    return 0;
} 
