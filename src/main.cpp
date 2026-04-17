// Windowsing, Input & Web Graphics
#define SDL_MAIN_HANDLED
#include "lib.h"

// Game, on PC we load the game dynamically (hot code reload)
// on WEB we include the game directly
#ifdef WEB_BUILD
#include "game.cpp"
#else 
#include "game.h"
void reload_game_dll();
typedef decltype(update_game) update_game_type;
static update_game_type* update_game_ptr;
#endif

// Web Rendering & testing
#ifdef WEB_BUILD
#include "emscripten/emscripten.h"
#endif
#include "gl_renderer.cpp"

// Texture Packer
#ifndef WEB_BUILD
#include <thread>
#include "texture_packer.cpp"
#endif

// #############################################################################
//                           Engine/Game Data
// #############################################################################
static GameState allocGameState = {};
static RenderData allocRenderData = {};

// #############################################################################
//                           Update Loop
// #############################################################################
void app_loop()
{
	static u64 ticksPerSecond = SDL_GetPerformanceFrequency();
  static const double secondsPerTick = 1.0 / ticksPerSecond;
	gameState->lastTick = gameState->currentTick;
	gameState->currentTick = SDL_GetPerformanceCounter();
	const float dt = (gameState->currentTick - gameState->lastTick) * secondsPerTick;

	#ifndef WEB_BUILD
	reload_game_dll();
	#endif

	// const float FPS = 1 / dt;
	// SDL_Log("FPS: %.0f", FPS);

	gameState->gameTime += dt;

	// Update Input/Window
	{
		// Reset input
		for(int i = 0; i < ACTION_COUNT; i++)
		{
			ActionToKey& actionToKey = gameState->input.keybinds[i];
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
				gameState->running = false;
			}
			else if(e.type == SDL_EVENT_WINDOW_RESIZED)
			{
    		SDL_GetWindowSize(gameState->window, &renderData->globalData.windowSize.x, &renderData->globalData.windowSize.y);
    		const Vec2& windowSize = vec_2(renderData->globalData.windowSize);
				renderData->viewportSize = windowSize;
				const float uiScale = renderData->uiScale;
				renderData->uiSpace = Vec2{ceilf(windowSize.x / uiScale), ceilf(windowSize.y / uiScale)};
			}
			else if(e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP)
			{
				for(int i = 0; i < ACTION_COUNT; i++)
				{
					ActionToKey& actionToKey = gameState->input.keybinds[i];
					if(e.button.button == actionToKey.key)
					{
						actionToKey.down = e.button.down;
						actionToKey.halfTransitionCount++;
					}
				}
			}
			//On keyboard key press
			else if(e.type == SDL_EVENT_KEY_DOWN)
			{
				for(int i = 0; i < ACTION_COUNT; i++)
				{
					ActionToKey& actionToKey = gameState->input.keybinds[i];
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
					ActionToKey& actionToKey = gameState->input.keybinds[i];
					if(e.key.key == actionToKey.key)
					{
						actionToKey.down = false;
						actionToKey.halfTransitionCount++;
					}
				}
			}
		}

		// Get Mouse Position
		SDL_GetMouseState(&input->mousePosScreen.x, &input->mousePosScreen.y);
		input->mousePosUI = window_to_ui(input->mousePosScreen);
	}

	// Update Game
	update_game(gameState, renderData, dt);
	update_ui();

	// Draw & Display
	gl_render(gameState->window);
	if(!SDL_GL_SwapWindow(gameState->window))
	{
		// I Hope you have a nice fucking day asshole
		// whoever wrote this I hope your mom has a nice 
		// birthday, fk you
 		ASSERT_MSG(false, "%s", SDL_GetError());
		gameState->running = false;
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

	#ifndef WEB_BUILD
  std::thread packTextureThread(pack_texture);
	#endif

	allocGameState = {};
	allocRenderData = {};
	gameState = &allocGameState;
	renderData = &allocRenderData;
  uiState = &gameState->uiState;
	input = &gameState->input;

	// Needed if performance for compiling takes 10mil years
	// like the dinosaurs, that was a cool time
	// memcpy(&gameState, &DEFAULT_GAMESTATE, sizeof(GameState));
	
	gameState->window = SDL_CreateWindow(WINDOW_TITLE, WORLD_SIZE.x, WORLD_SIZE.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(!gameState->window)
	{
		return -1;
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(gameState->window);
	SDL_SetWindowPosition(gameState->window, -1800, 200);
	#ifndef WEB_BUILD
	SDL_GL_SetSwapInterval(0); // Turn off VSync for PC, makes it lagg
	#endif

	// Renderer init
  SDL_GetWindowSize(gameState->window, &renderData->globalData.windowSize.x, &renderData->globalData.windowSize.y);
  const Vec2& windowSize = vec_2(renderData->globalData.windowSize);
	renderData->viewportSize = windowSize;
	const float uiScale = renderData->uiScale;
  renderData->uiSpace = Vec2{ceilf(windowSize.x / uiScale), ceilf(windowSize.y / uiScale)};
	gl_init(gameState->window);

	// If we don't do this, then the first calculation of dt
	// will result in over 190'000'.0f
	gameState->currentTick = SDL_GetPerformanceCounter();
	gameState->lastTick = gameState->currentTick;

	// Main Loop, on web we do emscripten stuff
	// on PC we just call app_loop
	#ifdef __EMSCRIPTEN__ 
	emscripten_set_main_loop(app_loop, 0, 0);
	#else 
	while (gameState->running) 
	{ 
		app_loop(); 
	} 
	#endif

	// Cleanup here after the program closes if needed
	#ifndef WEB_BUILD
  packTextureThread.detach();
	#endif

	return 0;
}

#ifndef WEB_BUILD
// #############################################################################
//                           Update Game implemented
// #############################################################################
void update_game(
  GameState* gameStateIn,
  RenderData* renderDataIn, 
  const float dt)
{
	update_game_ptr(gameStateIn, renderDataIn, dt);
}

// #############################################################################
//                           Reloading Game DLL
// #############################################################################
void reload_game_dll()
{
  static SDL_SharedObject* gameDLL;
  static long long lastTimestampGameDLL;

  long long currentTimestampGameDLL = get_timestamp("game.dll");
  if(currentTimestampGameDLL > lastTimestampGameDLL)
  {
    SDL_Log("Reloading game.dll");

    if(gameDLL)
    {
      SDL_UnloadObject(gameDLL);
      gameDLL = nullptr;
    }

    while(!SDL_CopyFile("game.dll", "game_load.dll"))
    {
      SDL_Delay(10);
    }
    SDL_Log("Copied game.dll to game_load.dll");

    gameDLL = SDL_LoadObject("game_load.dll");
    ASSERT_MSG(gameDLL, "Failed to load game_load.dll");

    update_game_ptr = (update_game_type*)SDL_LoadFunction(gameDLL, "update_game");
    ASSERT_MSG(update_game_ptr, "Failed to load update_game function");
    lastTimestampGameDLL = currentTimestampGameDLL;

    SDL_Log("Reloading game.dll done!");
  }
}

#include "dbghelp.h"

//Prints stack trace based on context record
int platform_print_stack(void* exPointer) 
{
  _EXCEPTION_POINTERS* ex = (_EXCEPTION_POINTERS*)exPointer;

  SDL_Log("*** Exception 0x%x occured ***\n", (unsigned int)ex->ExceptionRecord->ExceptionCode);    
  CONTEXT* ctx = ex->ContextRecord;

  const int MaxNameLen = 256;
  BOOL    result;
  HANDLE  process;
  HANDLE  thread;
  HMODULE hModule;

  STACKFRAME64        stack;
  ULONG               frame;    
  DWORD64             displacement;

  DWORD disp;
  IMAGEHLP_LINE64 *line;

  char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
  char name[MaxNameLen];
  char module[MaxNameLen];
  PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

  // On x64, StackWalk64 modifies the context record, that could
  // cause crashes, so we create a copy to prevent it
  CONTEXT ctxCopy;
  memcpy(&ctxCopy, ctx, sizeof(CONTEXT));

  memset( &stack, 0, sizeof( STACKFRAME64 ) );

  process                = GetCurrentProcess();
  thread                 = GetCurrentThread();
  displacement           = 0;
#if !defined(_M_AMD64)
  stack.AddrPC.Offset    = (*ctx).Eip;
  stack.AddrPC.Mode      = AddrModeFlat;
  stack.AddrStack.Offset = (*ctx).Esp;
  stack.AddrStack.Mode   = AddrModeFlat;
  stack.AddrFrame.Offset = (*ctx).Ebp;
  stack.AddrFrame.Mode   = AddrModeFlat;
#endif

  SymInitialize( process, NULL, TRUE ); //load symbols

  for( frame = 0; ; frame++ )
  {
    //get next call from stack
    result = StackWalk64
    (
#if defined(_M_AMD64)
      IMAGE_FILE_MACHINE_AMD64
#else
      IMAGE_FILE_MACHINE_I386
#endif
      ,
      process,
      thread,
      &stack,
      &ctxCopy,
      NULL,
      SymFunctionTableAccess64,
      SymGetModuleBase64,
      NULL);

    if( !result ) break;        

    //get symbol name for address
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;
    SymFromAddr(process, ( ULONG64 )stack.AddrPC.Offset, &displacement, pSymbol);

    line = (IMAGEHLP_LINE64 *)malloc(sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);       

    //try to get line
    if (SymGetLineFromAddr64(process, stack.AddrPC.Offset, &disp, line))
    {
      SDL_Log("\tat %s in %s: line: %lu: address: 0x%0X", 
               pSymbol->Name, line->FileName, line->LineNumber, 
               (unsigned int)pSymbol->Address);
    }
    else
    { 
      //failed to get line
      SDL_Log("\tat %s, address 0x%0X.", pSymbol->Name, (unsigned int)pSymbol->Address);
      hModule = NULL;
      lstrcpyA(module,"");        
      GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
          (LPCTSTR)(stack.AddrPC.Offset), &hModule);

      //at least print module name
      if(hModule != NULL)GetModuleFileNameA(hModule,module,MaxNameLen);       

      SDL_Log("in %s\n",module);
    }       

    free(line);
    line = NULL;
  }

  return EXCEPTION_EXECUTE_HANDLER;
}

#endif