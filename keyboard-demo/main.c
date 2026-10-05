#define GL_SILENCE_DEPRECATION   /* hides macOS "OpenGL is deprecated" warnings */
#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>          /*  includes the system OpenGL header */
#include "stb_easy_font.h"

#define MAX_TEXT 2000

static char   text[MAX_TEXT + 1];      /* everything the user has typed */
static size_t text_len = 0;
static char   last_key[160] = "Last key: (none yet)";

/* ---------- helpers ---------- */

static void append_char(char c)
{
    if (text_len < MAX_TEXT) {
        text[text_len++] = c;
        text[text_len] = '\0';
    }
}

/* Friendly names for keys that glfwGetKeyName() doesn't cover */
static const char* describe_key(int key, int scancode)
{
    const char* name = glfwGetKeyName(key, scancode);
    if (name) return name;

    switch (key) {
        case GLFW_KEY_SPACE:         return "Space";
        case GLFW_KEY_ENTER:         return "Enter";
        case GLFW_KEY_KP_ENTER:      return "Keypad Enter";
        case GLFW_KEY_BACKSPACE:     return "Backspace";
        case GLFW_KEY_TAB:           return "Tab";
        case GLFW_KEY_ESCAPE:        return "Escape";
        case GLFW_KEY_LEFT:          return "Left Arrow";
        case GLFW_KEY_RIGHT:         return "Right Arrow";
        case GLFW_KEY_UP:            return "Up Arrow";
        case GLFW_KEY_DOWN:          return "Down Arrow";
        case GLFW_KEY_LEFT_SHIFT:    return "Left Shift";
        case GLFW_KEY_RIGHT_SHIFT:   return "Right Shift";
        case GLFW_KEY_LEFT_CONTROL:  return "Left Ctrl";
        case GLFW_KEY_RIGHT_CONTROL: return "Right Ctrl";
        case GLFW_KEY_LEFT_ALT:      return "Left Alt";
        case GLFW_KEY_RIGHT_ALT:     return "Right Alt";
        case GLFW_KEY_CAPS_LOCK:     return "Caps Lock";
        default:                     return "Unknown";
    }
}

/* Draw a string at (x, y) in window coordinates, scaled up by `scale` */
static void draw_text(float x, float y, float scale, const char* str,
                      float r, float g, float b)
{
    static char buffer[1 << 20];   /* vertex data produced by stb_easy_font */
    int quads = stb_easy_font_print(0, 0, (char*)str, NULL, buffer, sizeof buffer);

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    glColor3f(r, g, b);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);   /* each vertex is 16 bytes */
    glDrawArrays(GL_QUADS, 0, quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

/* ---------- GLFW callbacks ---------- */

static void error_callback(int code, const char* description)
{
    fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

/* Called with actual text characters, with Shift, Caps Lock, and keyboard
   layout already applied. This is the right way to receive typed text. */
static void char_callback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    if (codepoint >= 32 && codepoint <= 126)
        append_char((char)codepoint);
    else
        append_char('?');   /* stb_easy_font only draws basic ASCII */
}

/* Called with physical key presses. Use it for keys that aren't text. */
static void key_callback(GLFWwindow* window, int key, int scancode,
                         int action, int mods)
{
    if (action == GLFW_RELEASE)
        return;   /* react to PRESS and REPEAT (key held down) */

    snprintf(last_key, sizeof last_key,
             "Last key: %s  (key=%d, scancode=%d)%s%s%s",
             describe_key(key, scancode), key, scancode,
             (mods & GLFW_MOD_SHIFT)   ? " +Shift" : "",
             (mods & GLFW_MOD_CONTROL) ? " +Ctrl"  : "",
             (mods & GLFW_MOD_ALT)     ? " +Alt"   : "");

    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_BACKSPACE:
            if (text_len > 0) text[--text_len] = '\0';
            break;
        case GLFW_KEY_ENTER:
        case GLFW_KEY_KP_ENTER:
            append_char('\n');
            break;
    }
}

/* ---------- main ---------- */

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return 1;

    GLFWwindow* window = glfwCreateWindow(900, 600, "GLFW Keyboard Demo", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);   /* vsync */

    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

    static char display[MAX_TEXT + 2];

    while (!glfwWindowShouldClose(window)) {
        int fb_w, fb_h, win_w, win_h;
        glfwGetFramebufferSize(window, &fb_w, &fb_h);   /* pixels */
        glfwGetWindowSize(window, &win_w, &win_h);      /* screen coords */

        if (win_w > 0 && win_h > 0) {   /* skip drawing while minimized */
            glViewport(0, 0, fb_w, fb_h);
            glClearColor(0.10f, 0.10f, 0.13f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            /* 2D projection: (0,0) is top-left, y grows downward */
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, win_w, win_h, 0, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            snprintf(display, sizeof display, "%s_", text);   /* add a cursor */

            draw_text(20, 20, 2.0f,
                      "Type something! Backspace deletes, Enter = new line, Esc quits.",
                      0.60f, 0.60f, 0.70f);
            draw_text(20, 50, 2.0f, last_key, 0.40f, 0.80f, 1.00f);
            draw_text(20, 100, 3.0f, display, 1.0f, 1.0f, 1.0f);

            glfwSwapBuffers(window);
        }

        glfwWaitEvents();   /* sleep until something happens */
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}