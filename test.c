#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Constants
#define WINDOW_W 800
#define WINDOW_H 600
#define NUM_STAGES 3
#define TRUE 1
#define FALSE 0

// Global Variables
int game_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;
SDL_Texture* stage_backgrounds[NUM_STAGES] = {NULL};

// Fonts
TTF_Font* title_font = NULL;
TTF_Font* gameplay_font = NULL;

// Game Modes
typedef enum {
    TITLE_SCREEN,
    PLAYING,
    GAME_OVER
} GameMode;

GameMode game_mode = TITLE_SCREEN;

// Stage Struct
typedef struct {
    int stage_number;
    int points_to_advance;
} Stage;

Stage stages[] = {
    {1, 10},
    {2, 20},
    {3, 30}
};

int current_stage = 1;
int player_points = 0;
int points_needed = 10;

// Function Prototypes
int initWindow(void);
int loadFont(void);
int loadAssets(void);
void processInput(void);
int setup(void);
void update(void);
void renderText(const char* text, int x, int y, SDL_Color color, TTF_Font* font);
int render(void);
void destroyWin(void);

// Initialize SDL Window and Renderer
int initWindow(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return FALSE;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "Error initializing SDL_ttf: %s\n", TTF_GetError());
        return FALSE;
    }

    window = SDL_CreateWindow("Cygnus x-1",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_W,
                              WINDOW_H,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Error creating SDL window: %s\n", SDL_GetError());
        return FALSE;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer: %s\n", SDL_GetError());
        return FALSE;
    }

    return TRUE;
}

// Load Fonts
int loadFont() {
    title_font = TTF_OpenFont("graphics/zalgo.ttf", 92);
    if (!title_font) {
        fprintf(stderr, "Failed to load title font: %s\n", TTF_GetError());
        return FALSE;
    }

    gameplay_font = TTF_OpenFont("graphics/zalgo.ttf", 32);
    if (!gameplay_font) {
        fprintf(stderr, "Failed to load gameplay font: %s\n", TTF_GetError());
        TTF_CloseFont(title_font);
        return FALSE;
    }

    return TRUE;
}

// Load Assets (Bitmaps)
int loadAssets() {
    // Load Title Screen
    SDL_Surface* surface = SDL_LoadBMP("graphics/title_screen.bmp");
    if (!surface) {
        fprintf(stderr, "Failed to load title_screen.bmp: %s\n", SDL_GetError());
        return FALSE;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        fprintf(stderr, "Failed to create texture for title screen: %s\n", SDL_GetError());
        return FALSE;
    }

    // Load Stage Backgrounds
    for (int i = 0; i < NUM_STAGES; i++) {
        char filename[50];
        sprintf(filename, "graphics/stage_%d.bmp", i + 1);

        surface = SDL_LoadBMP(filename);
        if (!surface) {
            fprintf(stderr, "Failed to load %s: %s\n", filename, SDL_GetError());
            return FALSE;
        }

        stage_backgrounds[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!stage_backgrounds[i]) {
            fprintf(stderr, "Failed to create texture for %s: %s\n", filename, SDL_GetError());
            return FALSE;
        }
    }

    return TRUE;
}

// Process Input
void processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game_running = FALSE;
        } else if (event.type == SDL_KEYDOWN) {
            if (game_mode == TITLE_SCREEN) {
                if (event.key.keysym.sym == SDLK_1) {
                    game_mode = PLAYING;
                    player_points = 0;
                    current_stage = 1;
                    points_needed = stages[current_stage - 1].points_to_advance;
                } else if (event.key.keysym.sym == SDLK_2) {
                    game_running = FALSE;
                }
            } else if (game_mode == PLAYING) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    game_running = FALSE;
                } else if (event.key.keysym.sym == SDLK_SPACE) {
                    player_points += 5; // Example for scoring mechanic
                }
            }
        }
    }
}

// Setup Game
int setup() {
    if (!loadFont() || !loadAssets()) {
        fprintf(stderr, "Failed to load assets during setup.\n");
        return FALSE;
    }

    return TRUE;
}

// Update Game Logic
void update() {
    if (game_mode == PLAYING && player_points >= points_needed) {
        current_stage++;
        player_points = 0;
        if (current_stage <= NUM_STAGES) {
            points_needed = stages[current_stage - 1].points_to_advance;
        } else {
            game_mode = GAME_OVER;
        }
    }
}

// Render Text
void renderText(const char* text, int x, int y, SDL_Color color, TTF_Font* font) {
    if (!text || !font) {
        fprintf(stderr, "Invalid text or font\n");
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);
    if (!textSurface) {
        fprintf(stderr, "Failed to create text surface: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture) {
        fprintf(stderr, "Failed to create text texture: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect dstRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);
    SDL_DestroyTexture(textTexture);
}

// Render Game Screen
int render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color color = {0, 255, 0, 255};
    if (game_mode == TITLE_SCREEN) {
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        renderText("Cygnus x-1", 200, 100, color, title_font);
    } else if (game_mode == PLAYING) {
        SDL_RenderCopy(renderer, stage_backgrounds[current_stage - 1], NULL, NULL);
        char buffer[100];
        sprintf(buffer, "Stage: %d Points: %d/%d", current_stage, player_points, points_needed);
        renderText(buffer, 50, 50, color, gameplay_font);
    }

    SDL_RenderPresent(renderer);
    return TRUE;
}

// Cleanup Resources
void destroyWin() {
    if (title_font) TTF_CloseFont(title_font);
    if (gameplay_font) TTF_CloseFont(gameplay_font);

    for (int i = 0; i < NUM_STAGES; i++) {
        if (stage_backgrounds[i]) SDL_DestroyTexture(stage_backgrounds[i]);
    }

    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();
}

// Main Function
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
