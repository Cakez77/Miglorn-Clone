// Windowsing, Input & Web Graphics
#define SDL_MAIN_HANDLED
#include "lib.h"

const Vec2 WORLD_SIZE = {1280, 720};

#ifdef WEB_BUILD
#include "emscripten/emscripten.h"
#endif
#include "gl_renderer.cpp"


#define WINDOW_TITLE "Window title"

// void draw_text_centered(const char* text, Vector2 pos, float fontSize)
// {
// 	const Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
//     pos.x -= textSize.x / 2.0f;
// 	DrawText(text, pos.x, pos.y, fontSize, WHITE);
// }

enum GameAction
{
	ACTION_PRIMARY,
	ACTION_SECONDARY,
	ACTION_LEFT,
	ACTION_UP,
	ACTION_DOWN,
	ACTION_RIGHT,

	ACTION_COUNT
};

struct ActionToKey
{
  union
  {
    SDL_Keycode key;
    SDL_MouseButtonFlags mouseBtn;
  };
  bool down;
  int halfTransitionCount;
};

struct GameOptions
{
  ActionToKey keybinds[ACTION_COUNT] = 
  {
    [ACTION_PRIMARY] = {.mouseBtn = SDL_BUTTON_LEFT},
    [ACTION_SECONDARY] = {.mouseBtn = SDL_BUTTON_RIGHT},
    [ACTION_LEFT] = {.key = SDLK_A},
    [ACTION_UP] = {.key = SDLK_W},
    [ACTION_DOWN] = {.key = SDLK_S},
    [ACTION_RIGHT] = {.key = SDLK_D},
  };
};

struct GameState
{
	uint64_t lastTick = 0;
	uint64_t currentTick = 0;
  GameOptions options;
	SDL_Window* window = nullptr;
	float gameTime;
	bool running = true;
  Vec2 playerPos;
};

const GameState DEFAULT_GAMESTATE
{
	.lastTick = 0,
	.currentTick = 0,
  .options = {},
	.window = nullptr,
	.gameTime = 0,
	.running = true,
  .playerPos = 0,
};

bool is_down(const GameState& gameState, GameAction gameAction)
{
  bool isDown = gameState.options.keybinds[gameAction].down;
  return isDown;
}

bool just_pressed(const GameState& gameState, GameAction gameAction)
{
  const ActionToKey& actionToKey = gameState.options.keybinds[gameAction];
  bool isDown = actionToKey.down;
  bool justPressed = (isDown && actionToKey.halfTransitionCount > 0) || (!isDown && actionToKey.halfTransitionCount > 1);
  return justPressed;
}

bool just_released(const GameState& gameState, GameAction gameAction)
{
  const ActionToKey& actionToKey = gameState.options.keybinds[gameAction];
  bool isUp = !actionToKey.down;
  bool justReleased = (isUp && actionToKey.halfTransitionCount > 0) || (!isUp && actionToKey.halfTransitionCount > 1);
  return justReleased;
}

// Init game
static GameState gameState = {};

void app_loop()
{
	static u64 ticksPerSecond = SDL_GetPerformanceFrequency();
  static const double secondsPerTick = 1.0 / ticksPerSecond;
	gameState.lastTick = gameState.currentTick;
	gameState.currentTick = SDL_GetPerformanceCounter();
	const float dt = (gameState.currentTick - gameState.lastTick) * secondsPerTick;

	// const float FPS = 1 / dt;
	// SDL_Log("FPS: %.0f", FPS);

	gameState.gameTime += dt;

	// Update Input/Window
	{
		// Reset input
		for(int i = 0; i < ACTION_COUNT; i++)
		{
			ActionToKey& actionToKey = gameState.options.keybinds[i];
			actionToKey.halfTransitionCount = 0;
		}

		// Gather Input using SDL
		SDL_Event e = {};
		while(SDL_PollEvent( &e ) == true)
		{
			//If event is quit type
			if(e.type == SDL_EVENT_QUIT)
			{
				//End the app loop
				gameState.running = false;
			}
			//On keyboard key press
			else if(e.type == SDL_EVENT_KEY_DOWN)
			{
				for(int i = 0; i < ACTION_COUNT; i++)
				{
					ActionToKey& actionToKey = gameState.options.keybinds[i];
					if(e.key.key == actionToKey.key)
					{
						actionToKey.down = true;
						actionToKey.halfTransitionCount++;
					}
				}
			}
			else if(e.type == SDL_EVENT_KEY_UP)
			{
				for(int i = 0; i < ACTION_COUNT; i++)
				{
					ActionToKey& actionToKey = gameState.options.keybinds[i];
					if(e.key.key == actionToKey.key)
					{
						actionToKey.down = false;
						actionToKey.halfTransitionCount++;
					}
				}
			}
		}
	}

	// Update and Draw Game
	{
		// Black
		// SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
		// SDL_RenderClear(renderer);

		Vec2& playerPos = gameState.playerPos;
		const float playerSpeed = 100;

		// Move Player
		{
			if(is_down(gameState, ACTION_LEFT))
			{
				playerPos.x -= playerSpeed * dt;
			}
			if(is_down(gameState, ACTION_UP))
			{
				playerPos.y -= playerSpeed * dt;
			}
			if(is_down(gameState, ACTION_DOWN))
			{
				playerPos.y += playerSpeed * dt;
			}
			if(is_down(gameState, ACTION_RIGHT))
			{
				playerPos.x += playerSpeed * dt;
			}
		}	

		// Setup Game Camera
		{
			// Game Camera
			{
				renderData.gameCam.dimensions = vec_2(renderData.globalData.windowSize);
				renderData.gameCam.position = gameState.playerPos;
				renderData.gameCam.zoom = 3;

				const OrthographicCamera2D& gameCam = renderData.gameCam;
				float zoom = gameCam.zoom? gameCam.zoom : 1.0f;
				Vec2 dimensions =  gameCam.dimensions / zoom;
				Vec2 pos = gameCam.position;

				renderData.globalData.orthProjGame =
					orthographic_projection(pos.x-dimensions.x / 2.0f,
																	pos.x+dimensions.x / 2.0f,
																	-pos.y-dimensions.y / 2.0f,
																	-pos.y+dimensions.y / 2.0f);
			}
		}
		// SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);  // Color of line
		float scale = sinf(gameState.gameTime) + 2;
		draw_sprite(SPRITE_SDV, {0, 0});
		draw_sprite(SPRITE_PLAYER, gameState.playerPos);
		// draw_sprite(SPRITE_PINE_TREE, {100, 200});
		// draw_sprite(SPRITE_PINE_TREE, {-300, 240});
		// draw_sprite(SPRITE_PINE_TREE, {160, 280});
		// draw_sprite(SPRITE_PINE_TREE, {-100, 220});
		// draw_sprite(SPRITE_PINE_TREE, {200, 150});

		draw_light(gameState.playerPos);

		// SDL_Surface* surface = TTF_RenderText_Blended(font, "Hello, SDL3!", 0, {255, 255, 255, 255});
		// if (surface) {
		//     auto text = SDL_CreateTextureFromSurface(renderer, surface);
		//     // text_w = (float)surface->w;
		//     // text_h = (float)surface->h;
		//     SDL_DestroySurface(surface);
		// }
		// TTF_CloseFont(font);


		// SDL_FRect playerRect = 
		// 		{gameState.playerPos.x - 20.0f, gameState.playerPos.y - 20.0f, 40.0f, 40.0f};
		// SDL_RenderFillRect(renderer, &playerRect);
		// SDL_RenderLine(renderer, playerPos.x, playerPos.y, playerPos.x + 100, playerPos.y);

		// SDL_RenderPresent(renderer);
	}

	// Draw
	float r = sinf(gameState.gameTime)/2.0f + 0.5f;
	gl_render(gameState.window, r);

	SDL_GL_SwapWindow(gameState.window);
}

int main(void)
{
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		return -1;
	}

	memcpy(&gameState, &DEFAULT_GAMESTATE, sizeof(GameState));

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	
	gameState.window = SDL_CreateWindow(WINDOW_TITLE, WORLD_SIZE.x, WORLD_SIZE.y, SDL_WINDOW_OPENGL);
	if(!gameState.window)
	{
		return -1;
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(gameState.window);
	SDL_SetWindowPosition(gameState.window, -1800, 200);
	#ifndef WEB_BUILD
	SDL_GL_SetSwapInterval(0); // Turn off VSync, makes it lagg
	#endif


	// Renderer init
	gl_init(gameState.window);

	#ifdef __EMSCRIPTEN__ 
	emscripten_set_main_loop(app_loop, 0, 0);
	#else 
	while (gameState.running) 
	{ 
		app_loop(); 
	} 
	#endif

	// SDL_DestroyRenderer(renderer);
	// SDL_DestroyWindow(gameState.window);
	// SDL_Quit();

	return 0;
}