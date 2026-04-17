#include "lib.h"
#include "input.h"
#include "ui.h"
#include "render_interface.h"

// #############################################################################
//                           Game Constants
// #############################################################################
constexpr char* WINDOW_TITLE = "Window title";
const Vec2 WORLD_SIZE = {1280, 720};

// #############################################################################
//                           Game Structs (Input)
// #############################################################################
struct GameState
{
	uint64_t lastTick = 0;
	uint64_t currentTick = 0;
	SDL_Window* window = nullptr;
	float gameTime;
  bool initialized = false;
	bool running = true;
  Vec2 playerPos;

  Input input;
  UIState uiState;
};

// #############################################################################
//                           Game Globals
// #############################################################################
static GameState* gameState;

// #############################################################################
//                           Game Functions, only one to make it easy
// #############################################################################
#ifndef WEB_BUILD
extern "C"
{
#endif

  EXPORT_FN void update_game(
    GameState* gameStateIn,
    RenderData* renderDataIn, 
    const float dt);

#ifndef WEB_BUILD
}
#endif








