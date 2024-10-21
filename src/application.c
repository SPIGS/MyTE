#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "application.h"

#define REGISTER_COMMAND(app, command) \
    applicationRegisterCommand(app, #command, Command_##command);

/* BEGIN GLFW CALLBACKS */

void key_callback(GLFWwindow* window, int key, int scancode, int action , int mods) {
    UNUSED(scancode);
    UNUSED(mods);

    Application *app = glfwGetWindowUserPointer(window);

    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        for (size_t i = 0; i < app->numKeybinds; ++i) {
            if (app->keybinds[i].key == key && app->keybinds[i].mods == mods) {
                app->keybinds[i].command(app);
                break;
            }
        }
    }

    switch (app->editor.mode) {
        case EDITOR_MODE_OPEN:
            applicationProcessBrowserInput(app, key, scancode, action, mods);
        break;
        
        default:
            applicationProcessEditorInput(app, key, scancode, action, mods);
        break;
    }
    
    // Global hotkeys 
    if (key == GLFW_KEY_F3 && (action == GLFW_PRESS)) {
        char *content = getContents(&app->editor);
        LOG_DEBUG("---------------------------\n%s", content);
        LOG_DEBUG("---------------------------", "");
        free(content);
    } else if (key == GLFW_KEY_F5 && (action == GLFW_PRESS)) {
        LOG_INFO("Reloading Config...", "");
        applicationReload(app);
        LOG_INFO("Config reloaded.", "");
        applicationSetStatusMessage(app, "Reloaded config.", 2.0f);
    }  else if (key == GLFW_KEY_F12 && (action == GLFW_PRESS)) {
        LOG_INFO("Clearing Buffer...", "");
        clearBuffer(&app->editor);
        applicationSetStatusMessage(app, "Cleared buffer.", 2.0f);
    } else if (key == GLFW_KEY_ESCAPE && (action == GLFW_PRESS)) {
        
    } 
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    Application *app = glfwGetWindowUserPointer(window);

    // TODO: keep track of the last character that the user entered,
    // If they reflexively try to complete these pairs, we should ignore
    // the second character
    if (app->editor.mode != EDITOR_MODE_OPEN) {
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
}

void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos) {
    Application *app = glfwGetWindowUserPointer(window);
    if (app->mouse_held) {
        moveCursorToMousePos(&app->editor, vec2_init(xpos, ypos));
        makeSelection(&app->editor);
    } 
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    Application *app = glfwGetWindowUserPointer(window);


    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        app->mouse_held = true;
        f64 mouse_x = 0.0;
        f64 mouse_y = 0.0;
        glfwGetCursorPos(app->window, &mouse_x, &mouse_y);
        moveCursorToMousePos(&app->editor, vec2_init(mouse_x, mouse_y));

        // Selection
        if ((mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT)
            makeSelection(&app->editor);
        else
            unselectSelection(&app->editor);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        app->mouse_held = false;
    }
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    UNUSED(xoffset);
    Application *app = glfwGetWindowUserPointer(window);
    scrollWithMouseWheel(&app->editor, yoffset);
}

void cursorEnterCallback(GLFWwindow *window, int entered) {
    Application *app = glfwGetWindowUserPointer(window);
    if (entered) {
        glfwSetCursor(app->window, glfwCreateStandardCursor(GLFW_IBEAM_CURSOR));
    } else {
        glfwSetCursor(app->window, NULL);
    }
}

void resize_window(GLFWwindow *window, int width, int height) {
   Application *app = glfwGetWindowUserPointer(window);
   rendererResizeWindow(&app->renderer, width, height);
}

void applicationRegisterCommand(Application *app, char *name, void (*command)()) {
    if (app->numCommands < MAX_COMMANDS) {
        app->commands[app->numCommands++] = (Command) { name, command };
    }
}

void applicationBindKey(Application *app, int key, int mods, const char *commandName) {
    for (size_t i = 0; i < app->numCommands; i++) {
        if (strcmp(app->commands[i].name, commandName) == 0) {
            app->keybinds[app->numKeybinds++] = (KeyBind){key, mods, app->commands[i].command};
            return;
        }
    }
    LOG_ERROR("Cannot bind command \'%s\' - command not found.", commandName);
}

/* END GLFW CALLBACKS */

void applicationInit(Application *app, int argc, char **argv) {
    app->status_message = NULL;
    app->status_disp_time = 0.0f;
    app->mouse_held = false;
    app->numCommands = 0;
    app->numKeybinds = 0;

    REGISTER_COMMAND(app, moveRight);
    REGISTER_COMMAND(app, selectRight);
    REGISTER_COMMAND(app, moveForwardWord);
    REGISTER_COMMAND(app, selectForwardWord);

    REGISTER_COMMAND(app, moveLeft);
    REGISTER_COMMAND(app, selectLeft);
    REGISTER_COMMAND(app, moveBackwardWord);
    REGISTER_COMMAND(app, selectBackwardWord);

    REGISTER_COMMAND(app, moveUp);
    REGISTER_COMMAND(app, selectUp);

    REGISTER_COMMAND(app, moveDown);
    REGISTER_COMMAND(app, selectDown);

    REGISTER_COMMAND(app, deleteLeft);
    REGISTER_COMMAND(app, deleteWordLeft);

    REGISTER_COMMAND(app, deleteRight);
    REGISTER_COMMAND(app, deleteWordRight);

    REGISTER_COMMAND(app, unselect);

    REGISTER_COMMAND(app, openBrowser);
    REGISTER_COMMAND(app, closeBrowser);
    REGISTER_COMMAND(app, write);

    // Initialize glfw
   if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW", "");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    app->window = glfwCreateWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "MyTE", NULL, NULL);
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
    LOG_INFO("OpenGL ver. %s", glGetString(GL_VERSION));

    app->config = configInit();
    loadConfigFromFile(&app->config, "./config/config.toml");

    // bind hotkeys
    for (size_t i = 0; i < app->config.numCommandConfigs; i++) {
        int key =  app->config.commandConfigs[i].key;
        int mods =  app->config.commandConfigs[i].mods;
        char *commandName = (char *)malloc(sizeof(char) * (strlen(app->config.commandConfigs[i].command_name) + 1));
        strcpy(commandName, app->config.commandConfigs[i].command_name);
        applicationBindKey(app, key, mods, commandName);
        free(commandName);
    }

    rendererInit(&app->renderer, COLOR_BLACK);
    app->font_id = rendererLoadFont(&app->renderer, app->config.font_path, app->config.font_size);
    rect editor_frame = rect_init(10, 0, INITIAL_SCREEN_WIDTH - 10, INITIAL_SCREEN_HEIGHT - 200);
    editorInit(&app->editor, editor_frame, app->renderer.font_atlases[app->font_id].atlas_height, app->renderer.glyph_adv, app->renderer.descender, ".");

    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, resize_window);
    glfwSetKeyCallback(app->window, key_callback);
    glfwSetCharCallback(app->window, character_callback);
    glfwSetMouseButtonCallback(app->window, mouseButtonCallback);
    glfwSetScrollCallback(app->window, scrollCallback);
    glfwSetCursorEnterCallback(app->window, cursorEnterCallback);
    glfwSetCursorPosCallback(app->window, mouseMoveCallback);
    
    if (argc > 1) {
        const char *file_path = argv[1];
        if (checkPath(file_path) == 0) {
            loadFromFile(&app->editor, file_path);
        } else if (checkPath(file_path) == 1) {
            editorDestroy(&app->editor);
            editorInit(&app->editor, editor_frame, app->renderer.font_atlases[app->font_id].atlas_height, app->renderer.glyph_adv, app->renderer.descender, file_path);
            editorChangeMode(&app->editor, EDITOR_MODE_OPEN);
        } else {
            LOG_ERROR("Error evaluating path. The file might be moved or missing!", "");
            applicationSetStatusMessage(app, "Error evaluating path. The file might be moved or missing!", 2.0f);
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

    if (app->status_message) {
        free(app->status_message);
    }

    glfwDestroyWindow(app->window);
    glfwTerminate();
}

void applicationReload(Application *app) {
    configDestroy(&app->config);
    app->config = configInit();
    loadConfigFromFile(&app->config, "./config/config.toml");
    app->font_id = rendererLoadFont(&app->renderer, app->config.font_path, app->config.font_size);
    
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
    editorInit(&app->editor, editor_frame, app->renderer.font_atlases[app->font_id].atlas_height, app->renderer.glyph_adv, app->renderer.descender, cur_dir);
    editorLoadConfig(&app->editor, &app->config);
    app->theme = colorThemeInit();
    if (app->config.theme_path)
        colorThemeLoad(&app->theme, app->config.theme_path);

    if (cur_file_path) {
        loadFromFile(&app->editor, cur_file_path);
    }
}

void applicationSetStatusMessage(Application *app, const char *msg, f32 t) {
    if (app->status_message) {
        free(app->status_message);
    }

    app->status_message = (char *)malloc(strlen(msg) * sizeof(char));
    strcpy(app->status_message, msg);
    app->status_disp_time = t;
}

void applicationUpdate(Application *app, f64 delta_time) {
    glfwPollEvents();
    editorUpdate(&app->editor, app->renderer.screen_width, app->renderer.screen_height, delta_time);
}

void applicationRender(Application *app, f64 delta_time) {
    rendererBegin(&app->renderer);
        
    // Render stuff goes here
    renderEditor(&app->renderer, app->font_id, &app->editor, delta_time, app->theme);

    // Draw the status message
    if (app->status_message && app->status_disp_time > 0.0f) {
        vec2 status_pos = vec2_init(app->editor.glyph_adv, 5.0);
        renderText(&app->renderer, app->status_message, &status_pos, &app->renderer.font_atlases[app->font_id], app->theme.foreground);
        app->status_disp_time -= (f32)delta_time;
    }

    // Draw FPS counter
    if (app->config.show_fps) {
        f64 fps = 1.0f / delta_time;
        char fps_str[200];
        sprintf(fps_str, "FPS: %f", fps);
        vec2 fps_pos = vec2_init(app->renderer.screen_width - (app->renderer.glyph_adv * 10.0), app->editor.line_height * 2);
        GlyphAtlas atlas = app->renderer.font_atlases[app->font_id];
        renderText(&app->renderer, fps_str, &fps_pos, &atlas, COLOR_RED);
    }

    rendererEnd(&app->renderer);
    glfwSwapBuffers(app->window);
}

void applicationProcessEditorInput (Application *app, int key, int scancode, int action , int mods) {
    UNUSED(scancode);
    UNUSED(mods);

    if (key == GLFW_KEY_ENTER && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        insertCharacter(&app->editor, '\n', true);
    } else if (key == GLFW_KEY_TAB && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        for (size_t i = 0; i< (size_t)app->editor.tab_stop; i++) {
            insertCharacter(&app->editor, ' ', true);
        }
    }
}

void applicationProcessBrowserInput (Application *app, int key, int scancode, int action , int mods) {
    UNUSED(scancode);
    UNUSED(mods);

    if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        FileBrowser *browser = &app->editor.browser;
        decrementSelection(browser);
    } else if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        FileBrowser *browser = &app->editor.browser;
        incrementSelection(browser);
    } else if (key == GLFW_KEY_ENTER && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        BrowserItem selection = getSelection(&app->editor.browser);
        if (selection.is_dir) {
            if (strcmp(selection.name_ext, "..") == 0) {
                goUpDirectoryLevel(&app->editor.browser);
                app->editor.browser.selection = 0;
                getPaths(&app->editor.browser);
            } else {
                enterDirectory(&app->editor.browser, selection.name_ext);
                app->editor.browser.selection = 0;
                getPaths(&app->editor.browser);
            }
        } else {
            char *cur_dir = (char *)malloc(sizeof(char) * strlen(app->editor.browser.cur_dir) + 1);
            strcpy(cur_dir, app->editor.browser.cur_dir);
            char *full_path = (char *)malloc(sizeof(char) * strlen(selection.full_path) + 1);
            strcpy(full_path, selection.full_path);

            editorDestroy(&app->editor);
            editorInit(&app->editor, INIT_EDITOR_FRAME, app->renderer.font_atlases[app->font_id].atlas_height, app->renderer.glyph_adv, app->renderer.descender, cur_dir);
            editorLoadConfig(&app->editor, &app->config);
            loadFromFile(&app->editor, full_path);
            free(cur_dir);
        }
    }
}

void Command_moveRight(Application *app) {
    moveCursorRight(&app->editor);
    unselectSelection(&app->editor);
}

void Command_moveForwardWord(Application *app) {
    moveEndOfNextWord(&app->editor);
    unselectSelection(&app->editor);
}

void Command_selectRight(Application *app) {
    moveCursorRight(&app->editor);
    makeSelection(&app->editor);
}

void Command_selectForwardWord(Application *app) {
    moveEndOfNextWord(&app->editor);
    makeSelection(&app->editor);
}

void Command_moveLeft(Application *app) {
    moveCursorLeft(&app->editor);
    unselectSelection(&app->editor);
}

void Command_moveBackwardWord(Application *app) {
    moveBegOfPrevWord(&app->editor);
    unselectSelection(&app->editor);
}

void Command_selectLeft(Application *app) {
    moveCursorLeft(&app->editor);
    makeSelection(&app->editor);
}

void Command_selectBackwardWord(Application *app) {
    moveBegOfPrevWord(&app->editor);
    makeSelection(&app->editor);
}

void Command_moveUp(Application *app) {
    moveCursorUp(&app->editor);
    unselectSelection(&app->editor);
}

void Command_selectUp(Application *app) {
    moveCursorUp(&app->editor);
    makeSelection(&app->editor);
}

void Command_moveDown(Application *app) {
    moveCursorDown(&app->editor);
    
}

void Command_selectDown(Application *app) {
    moveCursorDown(&app->editor);
    makeSelection(&app->editor);
}

void Command_deleteLeft(Application *app) {
    if (app->editor.cursor.selection_size != 0) {
        deleteSelection(&app->editor);
    } else {
        deleteCharacterLeft(&app->editor);
    }
}

void Command_deleteWordLeft(Application *app) {
    deleteWordLeft(&app->editor);
}

void Command_deleteRight(Application *app) {
    if (app->editor.cursor.selection_size != 0) {
        deleteSelection(&app->editor);
    } else {
        deleteCharacterRight(&app->editor);
    }
}

void Command_deleteWordRight(Application *app) {
    deleteWordRight(&app->editor);
}

void Command_openBrowser(Application *app) {
    editorChangeMode(&app->editor, EDITOR_MODE_OPEN);
}

void Command_unselect(Application *app) {
    unselectSelection(&app->editor);
}

void Command_closeBrowser(Application *app) {
    editorChangeMode(&app->editor, EDITOR_MODE_NORMAL);
}

void Command_write(Application *app) {
    writeToFile(&app->editor);
    applicationSetStatusMessage(app, "Saved to disk.", 2.0f);
}