#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>  /* handles platform-specific entry points */

#define WINDOW_W 800
#define WINDOW_H 600
#define TEXT_CAP 4096
#define SCALE    2.0f       /* the debug font is 8x8 px; scale it up */

static char   text[TEXT_CAP];
static size_t text_len = 0;
static char   last_key[128] = "Last key: (none yet)";

static void append_text(const char *s)
{
    size_t n = SDL_strlen(s);
    if (text_len + n < TEXT_CAP) {
        SDL_memcpy(text + text_len, s, n);
        text_len += n;
        text[text_len] = '\0';
    }
}

static void backspace(void)
{
    if (text_len == 0) return;
    /* Step back over UTF-8 continuation bytes so a whole character is removed */
    do {
        text_len--;
    } while (text_len > 0 && (text[text_len] & 0xC0) == 0x80);
    text[text_len] = '\0';
}

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window   *window   = NULL;
    SDL_Renderer *renderer = NULL;
    if (!SDL_CreateWindowAndRenderer("Keyboard Demo", WINDOW_W, WINDOW_H,
                                     SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Window/renderer creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);          /* cap frame rate to the display */
    SDL_SetRenderScale(renderer, SCALE, SCALE);
    SDL_StartTextInput(window);               /* enable SDL_EVENT_TEXT_INPUT */

    bool running = true;
    while (running) {
        /* ---- 1. Handle events ---- */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_TEXT_INPUT:
                append_text(event.text.text);
                break;

            case SDL_EVENT_KEY_DOWN:
                SDL_snprintf(last_key, sizeof(last_key), "Last key: %s%s",
                             SDL_GetKeyName(event.key.key),
                             event.key.repeat ? " (repeat)" : "");

                if (event.key.key == SDLK_BACKSPACE) {
                    backspace();
                } else if (event.key.key == SDLK_RETURN ||
                           event.key.key == SDLK_KP_ENTER) {
                    append_text("\n");
                } else if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                break;
            }
        }

        /* ---- 2. Draw ---- */
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 120, 200, 255, 255);
        SDL_RenderDebugText(renderer, 8, 8, last_key);

        /* The debug text doesn't handle '\n', so draw line by line */
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        static char line[TEXT_CAP + 2];
        const char *p = text;
        float y = 28;
        for (;;) {
            const char *nl = SDL_strchr(p, '\n');
            size_t n = nl ? (size_t)(nl - p) : SDL_strlen(p);
            SDL_memcpy(line, p, n);
            line[n] = '\0';
            if (!nl) SDL_strlcat(line, "_", sizeof(line));  /* cursor */
            SDL_RenderDebugText(renderer, 8, y, line);
            y += SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE + 2;
            if (!nl) break;
            p = nl + 1;
        }

        SDL_RenderPresent(renderer);
    }

    /* ---- 3. Clean up ---- */
    SDL_StopTextInput(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}