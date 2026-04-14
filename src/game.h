#include "lib.h"

// #############################################################################
//                           Game Constants
// #############################################################################
constexpr char* WINDOW_TITLE = "Window title";
const Vec2 WORLD_SIZE = {1280, 720};

// #############################################################################
//                           Game Structs (Input)
// #############################################################################
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
  float uiScale = 1.0f;
	bool running = true;
  Vec2 playerPos;
};

// #############################################################################
//                           Game Globals
// #############################################################################
static GameState gameState = {};

// #############################################################################
//                           Game Helper Functions
// #############################################################################
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

// #############################################################################
//                           Game Functions
// #############################################################################
bool init_game();
void update_game(const float dt);








