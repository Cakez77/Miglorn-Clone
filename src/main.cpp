// Windowsing, Input & Web Graphics
#define SDL_MAIN_HANDLED
#include "lib.h"

// Game
#include "game.cpp"

// Web Rendering & testing
#ifdef WEB_BUILD
#include "emscripten/emscripten.h"
#endif
#include "gl_renderer.cpp"


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
		while(SDL_PollEvent(&e) == true)
		{
			//If event is quit type
			if(e.type == SDL_EVENT_QUIT)
			{
				//End the app loop
				gameState.running = false;
			}
			else if(e.type == SDL_EVENT_WINDOW_RESIZED)
			{
    		SDL_GetWindowSize(gameState.window, &renderData.globalData.windowSize.x, &renderData.globalData.windowSize.y);
    		const Vec2& windowSize = vec_2(renderData.globalData.windowSize);
				const float uiScale = gameState.uiScale;
				renderData.uiSpace = Vec2{ceilf(windowSize.x / uiScale), ceilf(windowSize.y / uiScale)};
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

	// Update Game
	update_game(dt);

	// Draw & Display
	gl_render(gameState.window);
	if(!SDL_GL_SwapWindow(gameState.window))
	{
		// I Hope you have a nice fucking day asshole
		// whoever wrote this I hope your mom has a nice 
		// birthday, fk you
 		ASSERT_MSG(false, "%s", SDL_GetError());
		gameState.running = false;
	}
}

int main(void)
{
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	gameState = {};
	// Needed if performance for compiling takes 10mil years
	// like the dinosaurs, that was a cool time
	// memcpy(&gameState, &DEFAULT_GAMESTATE, sizeof(GameState));
	
	gameState.window = SDL_CreateWindow(WINDOW_TITLE, WORLD_SIZE.x, WORLD_SIZE.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
  SDL_GetWindowSize(gameState.window, &renderData.globalData.windowSize.x, &renderData.globalData.windowSize.y);
  const Vec2& windowSize = vec_2(renderData.globalData.windowSize);
	const float uiScale = gameState.uiScale;
  renderData.uiSpace = Vec2{ceilf(windowSize.x / uiScale), ceilf(windowSize.y / uiScale)};
	gl_init(gameState.window);

	// Game init
	init_game();

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