#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "application.h"

/* BEGIN GLFW CALLBACKS */

void key_callback(GLFWwindow* window, int key, int scancode, int action , int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    Application *app = glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        if (mods == GLFW_MOD_CONTROL) {
            moveCursorWordBackward(&app->editor);
        } else {
            moveCursorLeft(&app->editor);
        }
    } else if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        moveCursorUp(&app->editor);
    } else if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)){
       moveCursorDown(&app->editor);
    } else if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        if (mods == GLFW_MOD_CONTROL) {
            moveCursorWordForward(&app->editor);
        } else {
            moveCursorRight(&app->editor);
        }
    } else if (key == GLFW_KEY_BACKSPACE && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        deleteCharacterLeft(&app->editor);
    } else if (key == GLFW_KEY_DELETE && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        deleteCharacterRight(&app->editor);
    } else if (key == GLFW_KEY_ENTER && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        if (app->editor.mode != EDITOR_MODE_OPEN) {
            insertCharacter(&app->editor, '\n', true);
        } else {

            // TODO: This is garbage fix it
            BrowserItem selection = getSelection(&app->editor.browser);
            BrowserItem sel_copy;
            sel_copy.full_path = malloc(strlen(selection.full_path) + 1);
            sel_copy.name_ext = malloc(strlen(selection.name_ext) + 1);
            sel_copy.is_dir = selection.is_dir;
            strcpy(sel_copy.full_path, selection.full_path);
            strcpy(sel_copy.name_ext, selection.name_ext);
            if (sel_copy.is_dir) {
                if (strcmp(sel_copy.name_ext, "..") == 0) {
                    // leave the current directory
                    goUpDirectoryLevel(&app->editor.browser);
                } else {
                    enterDirectory(&app->editor.browser, sel_copy.name_ext);
                    app->editor.browser.selection = 0;
                    getPaths(&app->editor.browser);
                }
            } else {
                char *cur_dir = (char *)malloc(strlen(app->editor.browser.cur_dir) + 1);
                strcpy(cur_dir, app->editor.browser.cur_dir);
                editorDestroy(&app->editor);
                rect editor_frame = rect_init(10, 0, INITIAL_SCREEN_WIDTH - 10, INITIAL_SCREEN_HEIGHT - 200);
                editorInit(&app->editor, editor_frame, app->renderer.font_atlases[app->font_id].atlas_height, cur_dir);
                editorLoadConfig(&app->editor, &app->config);
                loadFromFile(&app->editor, sel_copy.full_path);
            }
        }
        
    } else if (key == GLFW_KEY_TAB && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        for (size_t i = 0; i< (size_t)app->editor.tab_stop; i++) {
            insertCharacter(&app->editor, ' ', true);
        }
    } else if (key == GLFW_KEY_F3 && (action == GLFW_PRESS)) {
        char *content = getContents(&app->editor);
        LOG_DEBUG("---------------------------\n", content);
        LOG_DEBUG("---------------------------", "");
        free(content);
    } else if (key == GLFW_KEY_F5 && (action == GLFW_PRESS)) {
        LOG_INFO("Reloading Config...", "");
        applicationReload(app);
        LOG_INFO("Config reloaded.", "");
    }  else if (key == GLFW_KEY_F12 && (action == GLFW_PRESS)) {
        LOG_INFO("Clearing Buffer...", "");
        clearBuffer(&app->editor);
    }  else if (key == GLFW_KEY_ESCAPE && (action == GLFW_PRESS)) {
        editorChangeMode(&app->editor, EDITOR_MODE_NORMAL);
    } else if (key == GLFW_KEY_O && (action == GLFW_PRESS)) {
        if (mods == GLFW_MOD_CONTROL)
            editorChangeMode(&app->editor, EDITOR_MODE_OPEN);
    } else if (key == GLFW_KEY_S && (action == GLFW_PRESS)) {
        if (mods == GLFW_MOD_CONTROL)
            writeToFile(&app->editor);
            //editorChangeMode(&app->editor, EDITOR_MODE_SAVE);
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    Application *app = glfwGetWindowUserPointer(window);

    // TODO: keep track of the last character that the user entered,
    // If they reflexively try to complete these pairs, we should ignore
    // the second character
    switch (codepoint) {
    case '{':
        insertCharacter(&app->editor, (char)codepoint, true);
        insertCharacter(&app->editor, '}', false);
        break;
    case '(':
        insertCharacter(&app->editor, (char)codepoint, true);
        insertCharacter(&app->editor, ')', false);
        break;
    case '\'':
        insertCharacter(&app->editor, (char)codepoint, true);
        insertCharacter(&app->editor, '\'', false);
        break;
    case '"':
        insertCharacter(&app->editor, (char)codepoint, true);
        insertCharacter(&app->editor, '"', false);
        break;
    case '[':
        insertCharacter(&app->editor, (char)codepoint, true);
        insertCharacter(&app->editor, ']', false);
        break;
    default:
        insertCharacter(&app->editor, (char)codepoint, true);
    }
}

void resize_window(GLFWwindow *window, int width, int height) {
	Application *app = glfwGetWindowUserPointer(window);
	rendererResizeWindow(&app->renderer, width, height);
}

/* END GLFW CALLBACKS */

void applicationInit(Application *app, int argc, char **argv) {
    // Initialize glfw
	if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW", "");
		exit(1);
	}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    app->window = glfwCreateWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "Hello World", NULL, NULL);
    if (!app->window) {
		glfwTerminate();
        LOG_ERROR("Failed to initialize GLFW window.", "");
		exit(1);
	}

    glfwMakeContextCurrent(app->window);

    if (glewInit() != GLEW_OK) {
        glfwTerminate();
		LOG_ERROR("Failed to initialize GLEW.", "");
		exit(1);
	}
    
    printf("OpenGL ver. %s\n", glGetString(GL_VERSION));

    app->config = configInit();
    loadConfigFromFile(&app->config, "./config/config.toml");

    rendererInit(&app->renderer, COLOR_BLACK);
    app->font_id = rendererLoadFont(&app->renderer, app->config.font_path, 24);
    rect editor_frame = rect_init(10, 0, INITIAL_SCREEN_WIDTH - 10, INITIAL_SCREEN_HEIGHT - 200);
    editorInit(&app->editor, editor_frame, app->renderer.font_atlases[app->font_id].atlas_height, ".");

    glfwSetWindowUserPointer(app->window, app);
	glfwSetFramebufferSizeCallback(app->window, resize_window);
	glfwSetKeyCallback(app->window, key_callback);
    glfwSetCharCallback(app->window, character_callback);

    if (argc > 1) {
        const char *file_path = argv[1];
        if (checkPath(file_path) == 0) {
            loadFromFile(&app->editor, file_path);
        } else if (checkPath(file_path) == 1) {
            editorDestroy(&app->editor);
            editorInit(&app->editor, editor_frame, app->renderer.font_atlases[app->font_id].atlas_height, file_path);
            editorChangeMode(&app->editor, EDITOR_MODE_OPEN);
        } else {
            LOG_ERROR("Error evaluating path. The file might be moved or missing!", "");
        }
    }

    app->theme = colorThemeInit();
    if (app->config.theme_path)
        colorThemeLoad(&app->theme, app->config.theme_path);

    editorLoadConfig(&app->editor, &app->config);
}

void applicationDestroy(Application *app) {
    rendererDestroy(&app->renderer);
    editorDestroy(&app->editor);
    configDestroy(&app->config);

    glfwDestroyWindow(app->window);
	glfwTerminate();
}

void applicationReload(Application *app) {
    configDestroy(&app->config);
    app->config = configInit();
    loadConfigFromFile(&app->config, "./config/config.toml");
    app->font_id = rendererLoadFont(&app->renderer, app->config.font_path, 24);
    
    // Save the current directory for the browser
    char *cur_dir = (char *)malloc(strlen(app->editor.browser.cur_dir) + 1);
    strcpy(cur_dir, app->editor.browser.cur_dir);
    
    // save the path to the current open file for the editor
    char *cur_file_path = NULL;
    if (app->editor.file_path) {
        cur_file_path = (char*)malloc(strlen(app->editor.file_path) + 1);
        strcpy(cur_file_path, app->editor.file_path);
    }
    
    editorDestroy(&app->editor);
    rect editor_frame = rect_init(10, 0, INITIAL_SCREEN_WIDTH - 10, INITIAL_SCREEN_HEIGHT - 200);
    editorInit(&app->editor, editor_frame, app->renderer.font_atlases[app->font_id].atlas_height, cur_dir);
    editorLoadConfig(&app->editor, &app->config);
    app->theme = colorThemeInit();
    if (app->config.theme_path)
        colorThemeLoad(&app->theme, app->config.theme_path);

    if (cur_file_path) {
        loadFromFile(&app->editor, cur_file_path);
    }
}

void applicationUpdate(Application *app, f64 delta_time) {
    glfwPollEvents();
    editorUpdate(&app->editor, app->renderer.screen_width, app->renderer.screen_height, app->theme, delta_time);
}

void applicationRender(Application *app, f64 delta_time) {
    rendererBegin(&app->renderer);
        
    // Render stuff goes here
    renderEditor(&app->renderer, app->font_id, &app->editor, delta_time, app->theme);
    
    // Draw FPS counter
    f64 fps = 1.0f / delta_time;
    char fps_str[200];
    sprintf(fps_str, "FPS: %f", fps);
    vec2 fps_pos = vec2_init(app->renderer.screen_width - 200, 30);
    renderText(&app->renderer, app->font_id, fps_str, &fps_pos, COLOR_RED);

    rendererEnd(&app->renderer);
    glfwSwapBuffers(app->window);
}