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

typedef enum
{
  TITLE_SCREEN,
  PLAYING,
  GAME_OVER
} GameMode;

GameMode game_mode = TITLE_SCREEN;

typedef struct
{
  int stage_number;
  int points_to_advance;
} Stage;

Stage stages[] = {
  {1,10},
  {2,20},
  {3,30}
};

// Function PROTOTYPES Bb-Z-z-i!
int initWindow(void);
int loadFont(void);
void processInput(void);
int setup(void);
void update(void);
void renderText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, TTF_Font* font);
int render(void);
void destoryWin(void);

// Init window and checking that TTF_Init worked
int initWindow(void)
{
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    fprintf(stderr, "Error initializing SDL.\n. Reason %s", SDL_GetError());
    return FALSE;
  }

  if (TTF_Init() != 0)
  {
    fprintf(stderr, "Error initializing SDL_ttf. Reason %s\n", TTF_GetError());
   return FALSE;
  }

  window = SDL_CreateWindow(NULL,
			    SDL_WINDOWPOS_CENTERED,
			    SDL_WINDOWPOS_CENTERED,
			    WINDOW_W,
			    WINDOW_H,
      			    SDL_WINDOW_SHOWN);
  //  SDL_Delay(3000);
  if (!window)
  {
    fprintf(stderr, "Error creating SDL window! %s\n", SDL_GetError());
    return FALSE;
  }

  renderer = SDL_CreateRenderer(window,	-1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer)
  {
    fprintf(stderr, "Error creating creating SDL renderer! %s\n", SDL_GetError());
    return FALSE;
  }
  
  return TRUE;
}


// Hopeless font-loader
int loadFont() {
  SDL_LockMutex(font_mutex);

  printf("Loading title font...\n");
  title_font = TTF_OpenFont("./graphics/zalgo.ttf", 92);
  if (!title_font) {
    fprintf(stderr, "Could not load title font from. Reason: %s\n", TTF_GetError());
    SDL_UnlockMutex(font_mutex);
    return FALSE;
  }
  printf("Title font loaded successfully %p\n", (void*)title_font);
   
  printf("Loading gameplay font...\n");
  gameplay_font = TTF_OpenFont("./graphics/zalgo.ttf", 32);
  if (!gameplay_font) {
    fprintf(stderr, "Could not load gameplay font from. Reason: %s\n", TTF_GetError());
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
int setup()
{
  game_mode = TITLE_SCREEN;
  surface = SDL_LoadBMP("./graphics/title_screen.bmp");
  if (!surface)
  {
    fprintf(stderr, "Could not load image %s\n", SDL_GetError());
    return FALSE;
  }

  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture)
  {
    fprintf(stderr, "Could not load texture");
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
      fprintf(stderr, "Could not load image %s: %s\n",
	      stage_images[i],
	      SDL_GetError());
      return FALSE;
    }
    stage_backgrounds[i] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free after texture created??

    if (!stage_backgrounds[i]) {
      fprintf(stderr, "Could not create texture for stage %d: %s\n",
	      i + 1, SDL_GetError());
      return FALSE;
    }
  }
  
  printf("Trying to pass SDL_CreateMutex to variable font_mutex");
  font_mutex = SDL_CreateMutex();
  if (font_mutex == NULL) {
    fprintf(stderr, "Could not create mutex: %s\n", SDL_GetError());
    return FALSE;
  }
  printf("Declaration off SDL_CreateMutex OK");

  if (!loadFont())
  {
    fprintf(stderr, "Failed to load font in setup()");
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
             player_points = 0; // Reset points
             current_stage = 1; // Start at stage 1
             points_needed = stages[current_stage - 1].points_to_advance;
         } else if (event.key.keysym.sym == SDLK_2) {
             printf("Exiting game!\n");
             game_running = FALSE;
         }
      } else if (game_mode == PLAYING) {
           if (event.key.keysym.sym == SDLK_1) {
             // Increment points for testing
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

  // Going to the next stage.
  if (player_points >= points_needed) {
    current_stage++;
    // TO-DO: add some logic for game over?
    player_points = 0;
    points_needed = stages[current_stage - 1].points_to_advance;
  }
}

// Hopeless text-renderer
void renderText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, TTF_Font* font)
{
  if (!text || !font || !renderer) {
    fprintf(stderr, "Invalid text, font or renderer: text=%p, font=%p\n", (void *)text, (void *)font);
    return; // Returns if text or font is null
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

  SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
  SDL_DestroyTexture(textTexture);
}

// Hopeless renderer
int render()
{
    SDL_RenderClear(renderer);

    SDL_Color malachite= {0, 255, 65, 255};
    SDL_Color islamic_green= {0, 143, 17, 255};
    SDL_Color dark_green= {0, 59, 0, 255};
    SDL_Color yellow= {238, 210, 2, 255};

    if (game_mode == TITLE_SCREEN) {
        SDL_RenderCopy(renderer, texture, NULL, NULL);
	
        SDL_LockMutex(font_mutex);
	// Check for title_font before rendering
        if (title_font != NULL) {
            renderText(renderer, "cygnus ... x-1", 460, 110, dark_green, title_font);
            renderText(renderer, "cygnus ... x-1", 455, 105, islamic_green, title_font);
            renderText(renderer, "cygnus ... x-1", 445, 95, yellow, title_font);
            renderText(renderer, "cygnus ... x-1", 450, 100, malachite, title_font);

            // Effect wizardry for flickering text
            delta_time = (SDL_GetTicks() - last_frame) / 1000.0;
            last_frame = SDL_GetTicks();
            // Makes for not to fast, not to slow flickering
            effect_inter = delta_time * 0.666;

            if (last_frame - last_effect >= effect_inter)
            {
                effect_state = !effect_state;
                last_effect = last_frame;
            }

            if (effect_state) {
                renderText(renderer, "0001 ....", 108, 480, dark_green, title_font);
                renderText(renderer, "0001 ....", 100, 485, islamic_green, title_font);
                renderText(renderer, "0001 ....", 91, 490, malachite, title_font);
                renderText(renderer, ".... 0010", 879, 480, dark_green, title_font);
                renderText(renderer, ".... 0010", 876, 485, islamic_green, title_font);
                renderText(renderer, ".... 0010", 867, 490, malachite, title_font);
            }
        } else {
            fprintf(stderr, "title_font is NULL in render().\n");
        }
	SDL_UnlockMutex(font_mutex);

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
            // Check for gameplay_font before rendering
            if (gameplay_font != NULL) {
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

int main(int argc, char *argv[])
{
  game_running = initWindow();
  setup();
  
  while (game_running)
  {
    processInput();
    update();
    render();
  }

  destroyWin();
}
