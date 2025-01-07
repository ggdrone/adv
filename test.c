// Cygnus x-1: work in progress

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <sys/stat.h>
#include "./files/consts.h"

// Too many globals
int           game_running = FALSE;
SDL_Window*   window       = NULL;
SDL_Renderer* renderer     = NULL;
SDL_Surface*  surface      = NULL;

// Title screen
SDL_Texture*  texture      = NULL;

// Stage backgrounds
SDL_Texture* stage_backgrounds[NUM_STAGES] = {NULL};

// Fonts, fonts, fonts
TTF_Font* title_font = NULL;
TTF_Font* gameplay_font = NULL;
SDL_mutex* font_mutex = NULL;

// Font effects
int           last_frame   = 0;
float         delta_time   = 0.0f;
int           effect_state = 1;
int           last_effect  = 0;
float         effect_inter = 0.0;

// Stage "logic" \("~")/
int          current_stage = 1;
int          player_points = 0;
int          points_needed = 10;

typedef enum {
    TITLE_SCREEN,
    PLAYING,
    GAME_OVER
} GameMode;

GameMode game_mode = TITLE_SCREEN;

typedef struct {
    int stage_number;
    int points_to_advance;
} Stage;

Stage stages[] = {
    {1, 10},
    {2, 20},
    {3, 30}
};

// Function PROTOTYPES
int initWindow(void);
int loadFont(void);
void processInput(void);
int setup(void);
void update(void);
void renderText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, TTF_Font* font);
void textEffects(SDL_Renderer* renderer);
int render(void);
void destroyWin(void);

// Init window and checking that TTF_Init worked
int initWindow(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL. Reason: %s\n", SDL_GetError());
        return FALSE;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "Error initializing SDL_ttf. Reason: %s\n", TTF_GetError());
        return FALSE;
    }

    window = SDL_CreateWindow(
        NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL window! Reason: %s\n", SDL_GetError());
        return FALSE;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer! Reason: %s\n", SDL_GetError());
        return FALSE;
    }

    return TRUE;
}

// Hopeless font-loader
int loadFont() {
    if (!font_mutex) {
        fprintf(stderr, "Font mutex is not initialized.\n");
        return FALSE;
    }

    SDL_LockMutex(font_mutex);

    printf("Loading title font...\n");
    title_font = TTF_OpenFont("./graphics/zalgo.ttf", 92);
    if (!title_font) {
        fprintf(stderr, "Failed to load title font. Reason: %s\n", TTF_GetError());
        SDL_UnlockMutex(font_mutex);
        return FALSE;
    }
    printf("Title font loaded successfully: %p\n", (void*)title_font);

    printf("Loading gameplay font...\n");
    gameplay_font = TTF_OpenFont("./graphics/zalgo.ttf", 32);
    if (!gameplay_font) {
        fprintf(stderr, "Failed to load gameplay font. Reason: %s\n", TTF_GetError());
        TTF_CloseFont(title_font);
        title_font = NULL;
        SDL_UnlockMutex(font_mutex);
        return FALSE;
    }
    printf("Gameplay font loaded successfully: %p\n", (void*)gameplay_font);

    SDL_UnlockMutex(font_mutex);

    return TRUE;
}

// Initial game setup, runs once
int setup() {
    game_mode = TITLE_SCREEN;
    surface = SDL_LoadBMP("graphics/title_screen.bmp");
    if (!surface) {
        fprintf(stderr, "Could not load image: %s\n", SDL_GetError());
        return FALSE;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        fprintf(stderr, "Could not create texture from surface. Reason: %s\n", SDL_GetError());
        return FALSE;
    }

    const char* stage_images[NUM_STAGES] = {
        "./graphics/stage_1.bmp",
        "./graphics/stage_2.bmp",
        "./graphics/stage_3.bmp"
    };

    for (int i = 0; i < NUM_STAGES; i++) {
        surface = SDL_LoadBMP(stage_images[i]);
        if (!surface) {
            fprintf(stderr, "Could not load image %s: %s\n", stage_images[i], SDL_GetError());
            return FALSE;
        }

        stage_backgrounds[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!stage_backgrounds[i]) {
            fprintf(stderr, "Could not create texture for stage %d: %s\n", i + 1, SDL_GetError());
            return FALSE;
        }
    }

    printf("Initializing font mutex...\n");
    font_mutex = SDL_CreateMutex();
    if (!font_mutex) {
        fprintf(stderr, "Failed to create mutex. Reason: %s\n", SDL_GetError());
        return FALSE;
    }
    printf("Font mutex initialized successfully.\n");

    if (!loadFont()) {
        fprintf(stderr, "Failed to load fonts during setup.\n");
        return FALSE;
    }

    return TRUE;
}

// Input processing for now
void processInput()
{
  SDL_Event event;
  SDL_PollEvent(&event);

  switch (event.type)
  {
  case SDL_QUIT:
      game_running = FALSE;
      break;
  case SDL_KEYDOWN:
      if (game_mode == TITLE_SCREEN) {
         if (event.key.keysym.sym == SDLK_1) {
             printf("Starting game!\n");
             game_mode = PLAYING;
             player_points = 0;
             current_stage = 1;
             points_needed = stages[current_stage - 1].points_to_advance;
         } else if (event.key.keysym.sym == SDLK_2) {
             printf("Exiting game!\n");
             game_running = FALSE;
         }
      } else if (game_mode == PLAYING) {
           if (event.key.keysym.sym == SDLK_1) {
             player_points += 1;
             printf("Points: %d\n", player_points);
         }
	   if (event.key.keysym.sym == SDLK_ESCAPE) {
	     game_running = FALSE;
	   }
      }
      break;
  }
}

// Updates screen
void update()
{
  if (game_mode != PLAYING) {
  return;
  }

  if (player_points >= points_needed) {
    current_stage++;
    player_points = 0;
    points_needed = stages[current_stage - 1].points_to_advance;
  }
}

// Render text with effects
void textEffects(SDL_Renderer* renderer) {
    if (title_font == NULL) {
        fprintf(stderr, "Error: title_font is NULL in textEffects().\n");
        return;
    }

    SDL_Color malachite = {0, 255, 65, 255};
    SDL_Color islamic_green = {0, 143, 17, 255};
    SDL_Color dark_green = {0, 59, 0, 255};
    SDL_Color yellow = {238, 210, 2, 255};

    renderText(renderer, "cygnus ... x-1", 460, 110, dark_green, title_font);
    renderText(renderer, "cygnus ... x-1", 455, 105, islamic_green, title_font);
    renderText(renderer, "cygnus ... x-1", 445, 95, yellow, title_font);
    renderText(renderer, "cygnus ... x-1", 450, 100, malachite, title_font);

    // Flickering text effect
    delta_time = (SDL_GetTicks() - last_frame) / 1000.0f;
    effect_inter = delta_time * 0.666f;

    if (SDL_GetTicks() - last_effect >= effect_inter) {
        effect_state = !effect_state;
        last_effect = SDL_GetTicks();
    }

    if (effect_state) {
        renderText(renderer, "0001 ....", 108, 480, dark_green, title_font);
        renderText(renderer, "0001 ....", 100, 485, islamic_green, title_font);
        renderText(renderer, "0001 ....", 91, 490, malachite, title_font);
        renderText(renderer, ".... 0010", 879, 480, dark_green, title_font);
        renderText(renderer, ".... 0010", 876, 485, islamic_green, title_font);
        renderText(renderer, ".... 0010", 867, 490, malachite, title_font);
    }
}

// Usage in render()


// Function to render text
void renderText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, TTF_Font* font) {
    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, color);
    if (!text_surface) {
        fprintf(stderr, "Error creating text surface: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (!text_texture) {
        fprintf(stderr, "Error creating text texture: %s\n", SDL_GetError());
        SDL_FreeSurface(text_surface);
        return;
    }

    SDL_Rect dst_rect = {x, y, text_surface->w, text_surface->h};
    SDL_RenderCopy(renderer, text_texture, NULL, &dst_rect);

    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

// Render function
int render()
{
    SDL_RenderClear(renderer);

    if (game_mode == TITLE_SCREEN) {
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        textEffects(renderer);
    } else if (game_mode == PLAYING) {
        if (current_stage - 1 < NUM_STAGES &&
            stage_backgrounds[current_stage - 1]) {
            SDL_RenderCopy(renderer,
                           stage_backgrounds[current_stage - 1],
                           NULL,
                           NULL);

            char stage_text[50];
            char points_text[50];

            sprintf(stage_text, "Stage: %d", current_stage);
            sprintf(points_text, "Points: %d .... %d", player_points, points_needed);

            SDL_LockMutex(font_mutex);
            if (gameplay_font != NULL) {
                SDL_Color malachite = {0, 255, 65, 255};
                renderText(renderer, stage_text, 5, 10, malachite, gameplay_font);
                renderText(renderer, points_text, 10, 50, malachite, gameplay_font);
            } else {
                fprintf(stderr, "gameplay_font is NULL in render().\n");
            }
            SDL_UnlockMutex(font_mutex);
        } else {
            fprintf(stderr, "Invalid stage or missing texture for stage %d\n",
                    current_stage);
        }
    }

    SDL_RenderPresent(renderer);
    return TRUE;
}

// Destroy everything created
void destroyWin()
{
  SDL_LockMutex(font_mutex);
  if (gameplay_font) TTF_CloseFont(gameplay_font);
  if (title_font) TTF_CloseFont(title_font);
  SDL_UnlockMutex(font_mutex);

  if (texture) SDL_DestroyTexture(texture);
  for (int i = 0; i < NUM_STAGES; i++) {
    if (stage_backgrounds[i]) {
      SDL_DestroyTexture(stage_backgrounds[i]);
    }
  }

  if (renderer) SDL_DestroyRenderer(renderer);
  if (window) SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
  SDL_DestroyMutex(font_mutex);

}

int main(int argc, char* argv[]) {
    if (!initWindow() || !setup()) {
        destroyWin();
        return EXIT_FAILURE;
    }

    game_running = TRUE;

    while (game_running) {
        processInput();
        update();
        render();
    }

    destroyWin();
    return EXIT_SUCCESS;
}
