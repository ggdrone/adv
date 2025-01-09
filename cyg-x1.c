/* CYGNUS X-1 */

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define TRUE 1
#define FALSE 0
#define WINDOW_W 1200
#define WINDOW_H 600
#define NUM_STAGES 3
#define NUM_FONTS 2
#define NUM_COLORS 4

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Prototypes
int initSDL(SDL_Window** window, SDL_Renderer** renderer);
/*------------------------------------------------------------------------*/
SDL_Texture* load_texture(const char* file_path, SDL_Renderer* renderer);
/*------------------------------------------------------------------------*/
TTF_Font* load_font(const char* file_path, int font_size);
/*------------------------------------------------------------------------*/
int game_setup(SDL_Window* window, SDL_Texture** texture, SDL_Texture* stage_backgrounds[], 
            SDL_Renderer* renderer, TTF_Font* fonts[], SDL_Color colors[], int* effect_state);
/*------------------------------------------------------------------------*/
void renderText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color,
		TTF_Font* font);
/*------------------------------------------------------------------------*/
int render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color* colors, TTF_Font** fonts,
	   int* effect_state);
/*------------------------------------------------------------------------*/
void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, 
            SDL_Texture* stage_backgrounds[], TTF_Font* fonts[], int num_fonts);
/*------------------------------------------------------------------------*/

// Init SDL and TTF
int initSDL(SDL_Window** window, SDL_Renderer** renderer) {
  printf("\n----- Entering initSDL and TTF -----\n");
  printf("Checking if SDL initialized...");
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Could not init SDL. Reason: %s\n", SDL_GetError());
    return FALSE;
  }
  printf("SDL initialization successful!\n");

  printf("Checking if TTF initialized...");
  if (TTF_Init() != 0) {
    fprintf(stderr, "Could not init TTF. Reason: %s\n", TTF_GetError());
    return FALSE;
  }
  printf("TTF initialization successful!\n");

  printf("Creating SDL Window...");
  *window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			     WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);

  if (!*window) {
    fprintf(stderr, "Could not load SDL_Window. Reason SDL: %s\n", SDL_GetError());
    return FALSE;
  }
  printf("SDL Window successfully created!\n");

  printf("Creating SDL Renderer...");
  *renderer = SDL_CreateRenderer(*window, -1,
				 SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!*renderer) {
    fprintf(stderr, "Could not load SDL_Renderer. Reason SDL: %s\n", SDL_GetError());
    printf("Destroying window and quitting!\n");
    SDL_DestroyWindow(*window);
    SDL_Quit();
    return FALSE;
  }
  printf("SDL Renderer successfully created!\n");

  return TRUE;
}

// Helper function for game_setup(), loads backgrounds images
SDL_Texture* load_texture(const char* file_path, SDL_Renderer* renderer) {
  SDL_Surface* surface = SDL_LoadBMP(file_path);
  if (!surface) {
    fprintf(stderr, "Failed to load BMP %s: %s\n", file_path, SDL_GetError());
    return NULL;
  }
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  if (!texture) {
    fprintf(stderr, "Failed to create texture from %s: %s\n", file_path, SDL_GetError());
  }
  return texture;
}

//Helper function for game_setup(), loads fonts
TTF_Font* load_font(const char* file_path, int font_size) {
  TTF_Font* font = TTF_OpenFont(file_path, font_size);
  if (!font) {
    fprintf(stderr, "Could not load font at %s. Reason: %s\n", file_path, TTF_GetError());
    return NULL;
  }
  return font;
}

// Big bad loader function
int game_setup(SDL_Window* window, SDL_Texture** texture, SDL_Texture* stage_backgrounds[], 
            SDL_Renderer* renderer, TTF_Font* fonts[], SDL_Color colors[],
            int* effect_state) {
  printf("\n----- Entering game_setup -----\n");
  printf("Setting up title screen background...");

  *texture = load_texture("./graphics/title_screen.bmp", renderer);
  if (!*texture) {
    fprintf(stderr, "Failed to set up title screen background. Exiting...\n");
    return FALSE;
  }
  printf("Successfully set up title screen background!\n");

  printf("Setting up stage backgrounds...");
  const char* stage_images[NUM_STAGES] = {
    "./graphics/stage_1.bmp",
    "./graphics/stage_2.bmp",
    "./graphics/stage_3.bmp"
  };

  for (int i = 0; i < NUM_STAGES; i++) {
    stage_backgrounds[i] = load_texture(stage_images[i], renderer);
    if (!stage_backgrounds[i]) {
      fprintf(stderr, "Failed to load stage background %s. Cleaning up...\n",
	      stage_images[i]);
      for (int j = 0; j < i; j++) {
	SDL_DestroyTexture(stage_backgrounds[j]);
      }
      SDL_DestroyTexture(*texture);
      return FALSE;
      }
  }
  printf("Successfully set up all stage backgrounds!\n");

  printf("Setting up fonts...");
  fonts[0] = load_font("./graphics/zalgo.ttf", 92); // Title font
  fonts[1] = load_font("./graphics/zalgo.ttf", 32); // Small font
  if (!fonts[0] || !fonts[1]) {
    fprintf(stderr, "Could not load fonts. Reason: %s\n", TTF_GetError());
    cleanup(window, renderer, *texture, stage_backgrounds, fonts, NUM_FONTS);
    return FALSE;
  }
  printf("Successfully set up fonts!\n");

  printf("Setting up colors...");
  colors[0] = (SDL_Color){0, 255, 65, 255};     // Malachite
  colors[1] = (SDL_Color){0, 143, 17, 255};    // Islamic green
  colors[2] = (SDL_Color){0, 59, 0, 255};      // Dark green
  colors[3] = (SDL_Color){238, 210, 2, 255};   // Yellow
  printf("Colors set up successfully!\n");

  printf("Setting up effect variables...\n");
  *effect_state = 1;
  printf("Effect variables initialized successfully!\n");

  return TRUE;
}

// Textrender function 
void renderText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, TTF_Font* font) {
  SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, color);
  if (!text_surface) {
    fprintf(stderr, "Could not render text to surface! Reason: %s\n", TTF_GetError());
    return;
  }

  SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
  SDL_FreeSurface(text_surface);
  if (!text_texture) {
    fprintf(stderr, "Could not create text texture! Reason: %s\n", SDL_GetError());
    return;
  }

  SDL_Rect dst_rect = {x, y, 0, 0};
  SDL_QueryTexture(text_texture, NULL, NULL, &dst_rect.w, &dst_rect.h);
  SDL_RenderCopy(renderer, text_texture, NULL, &dst_rect);
  SDL_DestroyTexture(text_texture);
}

// Renderer baby
int render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color* colors, TTF_Font** fonts,
        int* effect_state) {

  if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) {
    fprintf(stderr, "Could not render draw color. Reason: %s\n", SDL_GetError());
    return FALSE;
  }

  if (SDL_RenderClear(renderer)) {
    fprintf(stderr, "Could not clear renderer. Reason: %s\n", SDL_GetError());
    return FALSE;
  }

  if (SDL_RenderCopy(renderer, texture, NULL, NULL)) {
    fprintf(stderr, "Could not render copy. Reason: %s\n", SDL_GetError());
    return FALSE;
  }

  // Effect wizardry
  static int last_frame = 0;
  static int last_effect = 0;
  float delta_time = (SDL_GetTicks() - last_frame) / 1000.0f;
  last_frame = SDL_GetTicks();
  float effect_inter = delta_time * 0.666f; //Evil number of no reason

  if (last_frame - last_effect >= effect_inter) {
    *effect_state = !(*effect_state);
    last_effect = last_frame;
  }
  renderText(renderer, "cygnus ... x-1", 460, 110, colors[2], fonts[0]);
  renderText(renderer, "cygnus ... x-1", 455, 105, colors[1], fonts[0]);
  renderText(renderer, "cygnus ... x-1", 445, 95, colors[3], fonts[0]);
  renderText(renderer, "cygnus ... x-1", 450, 100, colors[0], fonts[0]);
  // Render text if effect state is active
  if (*effect_state) {

    renderText(renderer, "0001 ....", 108, 480, colors[2], fonts[0]);
    renderText(renderer, "0001 ....", 100, 485, colors[1], fonts[0]);
    renderText(renderer, "0001 ....", 91, 490, colors[0], fonts[0]);
    renderText(renderer, ".... 0010", 879, 480, colors[2], fonts[0]);
    renderText(renderer, ".... 0010", 876, 485, colors[1], fonts[0]);
    renderText(renderer, ".... 0010", 867, 490, colors[0], fonts[0]);
  }
  SDL_RenderPresent(renderer);
  return TRUE;
}

// Cleanup after use
void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, SDL_Texture* stage_backgrounds[], TTF_Font* fonts[], int num_fonts) {
  printf("\n---- Entering cleanup ----\n");

  printf("Killing stage backgrounds...\n");
  for (int i = 0; i < NUM_STAGES; i++) {
    if (stage_backgrounds[i]) {
      SDL_DestroyTexture(stage_backgrounds[i]);
      printf("Success! Killed stage background %d\n", i + 1);
    } else {
      printf("Stage background %d already NULL.\n", i + 1);
    }
  }

  printf("Killing title background...");
  if (texture) {
    SDL_DestroyTexture(texture);
    printf("Success! Killed title background\n");
  } else {
    printf("Title background already NULL.\n");
  }

  printf("Killing fonts...\n");
  for (int i = 0; i < num_fonts; i++) {
    if (fonts[i]) {
      TTF_CloseFont(fonts[i]);
      printf("Success! Killed font %d\n", i + 1);
    } else {
      printf("Font %d already NULL.\n", i + 1);
    }
  }

  printf("Killing renderer...");
  if (renderer) {
    SDL_DestroyRenderer(renderer);
    printf("Success! Killed renderer\n");
  } else {
    printf("SDL Renderer already NULL.\n");
  }

  printf("Killing window...");
  if (window) {
    SDL_DestroyWindow(window);
    printf("Success! Killed window\n");
  } else {
    printf("SDL Window already NULL.\n");
  }

  TTF_Quit();
  SDL_Quit();
  printf("All cleaned up! Quitting.\n");
}

int main(void) {
  
  printf("---- Starting program ----\n");
  printf("---- Cygnus       x-1 ----\n");
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  SDL_Texture* texture = NULL;
  SDL_Texture* stage_backgrounds[NUM_STAGES] = {NULL};
  TTF_Font* fonts[NUM_FONTS] = {NULL};
  SDL_Color colors[NUM_COLORS];
  int effect_state = 1;

  printf("Initializing SDL and renderer...\n");
  if (!initSDL(&window, &renderer)) {
      printf("Initialization failed. Exiting...\n");
      return EXIT_FAILURE;
  }
  printf("SDL and renderer initialized successfully!\n");

  printf("Setting up game assets...\n");
  if (!game_setup(window, &texture, stage_backgrounds, renderer, fonts, colors, &effect_state)) {
      printf("Game setup failed. Exiting...\n");
      cleanup(window, renderer, texture, stage_backgrounds, fonts, NUM_FONTS);
      return EXIT_FAILURE;
  }
  printf("Game assets set up successfully!\n");

  printf("\n---- Entering game loop ----\n");
  SDL_Event event;
  int game_running = TRUE;
  while (game_running) {
      while (SDL_PollEvent(&event)) {
	  if (event.type == SDL_QUIT) game_running = FALSE;
      }
      if (!render(renderer, texture, colors, fonts, &effect_state)) {
	  fprintf(stderr, "Render error occurred. Exiting loop...\n");
	  break;
      }
  }

  printf("Exiting game loop.\n");
  cleanup(window, renderer, texture, stage_backgrounds, fonts, NUM_FONTS);
  return EXIT_SUCCESS;
}
