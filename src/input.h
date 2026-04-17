#pragma once

#include "lib.h"

// #############################################################################
//                           Input Structs
// #############################################################################
enum GameAction
{
	ACTION_PRIMARY,
	ACTION_SECONDARY,
	ACTION_LEFT,
	ACTION_UP,
	ACTION_DOWN,
	ACTION_RIGHT,
	ACTION_SCALE_UP,
	ACTION_SCALE_DOWN,

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

struct Input
{
  Vec2 mousePosScreen;
  Vec2 mousePosUI;
  Vec2 mousePosWorld;

  // This will be loaded from a config.ini file
  ActionToKey keybinds[ACTION_COUNT] = 
  {
    [ACTION_PRIMARY] = {.mouseBtn = SDL_BUTTON_LEFT},
    [ACTION_SECONDARY] = {.mouseBtn = SDL_BUTTON_RIGHT},
    [ACTION_LEFT] = {.key = SDLK_A},
    [ACTION_UP] = {.key = SDLK_W},
    [ACTION_DOWN] = {.key = SDLK_S},
    [ACTION_RIGHT] = {.key = SDLK_D},
    [ACTION_SCALE_UP] = {.key = SDLK_R},
    [ACTION_SCALE_DOWN] = {.key = SDLK_F},
  };
};

// #############################################################################
//                           Input Structs
// #############################################################################
static Input* input;

// #############################################################################
//                           Input Functions
// #############################################################################
bool is_down(GameAction gameAction)
{
  bool isDown = input->keybinds[gameAction].down;
  return isDown;
}

bool just_pressed(GameAction gameAction)
{
  const ActionToKey& actionToKey = input->keybinds[gameAction];
  bool isDown = actionToKey.down;
  bool justPressed = (isDown && actionToKey.halfTransitionCount > 0) || (!isDown && actionToKey.halfTransitionCount > 1);
  return justPressed;
}

bool just_released(GameAction gameAction)
{
  const ActionToKey& actionToKey = input->keybinds[gameAction];
  bool isUp = !actionToKey.down;
  bool justReleased = (isUp && actionToKey.halfTransitionCount > 0) || (!isUp && actionToKey.halfTransitionCount > 1);
  return justReleased;
}

bool consume_just_pressed(GameAction action)
{
  if(just_pressed(action))
  {
    ActionToKey& actionToKey = input->keybinds[action];
    if(actionToKey.halfTransitionCount > 1)
    {
      actionToKey.down = !actionToKey.down;
    }

    // I feel like this still needs to happen?
    // actionToKey.justPressed = false;
    actionToKey.halfTransitionCount--;
    return true;
  }
  return false;
}

bool consume_just_released(GameAction action)
{
  if(just_released(action))
  {
    ActionToKey& actionToKey = input->keybinds[action];
    if(actionToKey.halfTransitionCount > 1)
    {
      actionToKey.down = !actionToKey.down;
    }

    // I feel like this still needs to happen?
    // actionToKey.justReleased = false;
    actionToKey.halfTransitionCount--;
    return true;
  }
  return false;
}