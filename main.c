#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "font.h"
#include "editor.h"
#include "theme.h"

// TODO: Move this into an application struct
Editor editor;
Renderer renderer;
Config config;
ColorTheme theme;
u32 font_id;

void key_callback(GLFWwindow* window, int key, int scancode, int action , int mods)
{
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(mods);

    if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        if (mods == GLFW_MOD_CONTROL) {
            moveCursorWordBackward(&editor);
        } else {
            moveCursorLeft(&editor);
        }
    } else if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        moveCursorUp(&editor);
    } else if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)){
       moveCursorDown(&editor);
    } else if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        if (mods == GLFW_MOD_CONTROL) {
            moveCursorWordForward(&editor);
        } else {
            moveCursorRight(&editor);
        }
    } else if (key == GLFW_KEY_BACKSPACE && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        deleteCharacterLeft(&editor);
    } else if (key == GLFW_KEY_DELETE && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        deleteCharacterRight(&editor);
    } else if (key == GLFW_KEY_ENTER && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        insertCharacter(&editor, '\n', true);
    } else if (key == GLFW_KEY_TAB && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        for (size_t i = 0; i< (size_t)editor.tab_stop; i++) {
            insertCharacter(&editor, ' ', true);
        }
    } else if (key == GLFW_KEY_F3 && (action == GLFW_PRESS)) {
        printf("---------------------------\n");
        char *content = getContents(&editor);
        printf("%s\n", content);
        free(content);
    } else if (key == GLFW_KEY_F5 && (action == GLFW_PRESS)) {
        LOG_INFO("Reloading Config...", "");
        configDestroy(&config);
        config = configInit();
        loadConfigFromFile(&config, "./config/config.toml");
        font_id = rendererLoadFont(&renderer, config.font_path, 24);
        char *file_path = malloc(strlen(editor.file_path) + 1);
        strcpy(file_path, editor.file_path);
        editorDestroy(&editor);
        rect editor_frame = rect_init(10, 0, INITIAL_SCREEN_WIDTH - 10, INITIAL_SCREEN_HEIGHT - 200);
        editorInit(&editor, editor_frame, renderer.font_atlases[font_id].atlas_height);
        loadFromFile(&editor, file_path);
        editorLoadConfig(&editor, &config);
        theme = colorThemeInit();
        if (config.theme_path)
            colorThemeLoad(&theme, config.theme_path);

        LOG_INFO("Config reloaded.", "");
    }  else if (key == GLFW_KEY_F12 && (action == GLFW_PRESS)) {
        LOG_INFO("Clearing Buffer...", "");
        clearBuffer(&editor);
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    UNUSED(window);

    // TODO: keep track of the last character that the user entered,
    // If they reflexively try to complete these pairs, we should ignore
    // the second character
    switch (codepoint) {
    case '{':
        insertCharacter(&editor, (char)codepoint, true);
        insertCharacter(&editor, '}', false);
        break;
    case '(':
        insertCharacter(&editor, (char)codepoint, true);
        insertCharacter(&editor, ')', false);
        break;
    case '\'':
        insertCharacter(&editor, (char)codepoint, true);
        insertCharacter(&editor, '\'', false);
        break;
    case '"':
        insertCharacter(&editor, (char)codepoint, true);
        insertCharacter(&editor, '"', false);
        break;
    case '[':
        insertCharacter(&editor, (char)codepoint, true);
        insertCharacter(&editor, ']', false);
        break;
    default:
        insertCharacter(&editor, (char)codepoint, true);
    }
}

void resize_window(GLFWwindow *window, int width, int height) {
	Renderer *r = glfwGetWindowUserPointer(window);
	rendererResizeWindow(r, width, height);
}

int main (int argc, char **argv) {
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

    config = configInit();
    loadConfigFromFile(&config, "./config/config.toml");

    rendererInit(&renderer, COLOR_BLACK);
	font_id = rendererLoadFont(&renderer, config.font_path, 24);
    rect editor_frame = rect_init(10, 0, INITIAL_SCREEN_WIDTH - 10, INITIAL_SCREEN_HEIGHT - 200);
    editorInit(&editor, editor_frame, renderer.font_atlases[font_id].atlas_height);

	glfwSetWindowUserPointer(window, &renderer);
	glfwSetFramebufferSizeCallback(window, resize_window);
	glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);

    if (argc > 1) {
        const char *file_path = argv[1];
        if (isFile(file_path)) {
            loadFromFile(&editor, file_path);
        } else {
            printf("WARN: Error evaluating path. The file might be moved or missing or a directory (not supported)!\n");
        }
    }

    theme = colorThemeInit();
    if (config.theme_path)
        colorThemeLoad(&theme, config.theme_path);

    editorLoadConfig(&editor, &config);

    f64 last_frame_time = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        f64 cur_fame_time = (f64)glfwGetTime();
        f64 delta_time = cur_fame_time - last_frame_time;
        last_frame_time = cur_fame_time;

        glfwPollEvents();
        editorUpdate(&editor, renderer.screen_width, renderer.screen_height, theme, delta_time);
        rendererBegin(&renderer);
        
		// Render stuff goes here
        renderEditor(&renderer, font_id, &editor, delta_time, theme);
        f64 fps = 1.0f / delta_time;
        char fps_str[200];
        sprintf(fps_str, "FPS: %f", fps);
        vec2 fps_pos = vec2_init(renderer.screen_width - 200, 30);
        renderText(&renderer, font_id, fps_str, &fps_pos, COLOR_RED);

        rendererEnd(&renderer);
        glfwSwapBuffers(window);
    }
    rendererDestroy(&renderer);
    editorDestroy(&editor);
    configDestroy(&config);

    glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
}