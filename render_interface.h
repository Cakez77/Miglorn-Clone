#pragma once
#include "lib.h"
#include "assets.h"

static SDL_Renderer* renderer;
static SDL_Texture* atlas;

// #############################################################################
//                           Render Constats
// #############################################################################
constexpr u32 MAX_TRANSFORMS = 1000;

// #############################################################################
//                           Render Structs
// #############################################################################
struct OrthographicCamera2D
{
  float zoom = 1.0f;
  Vec2 dimensions;
  Vec2 position;
};

struct Transform
{
  Vec2 atlasPos;
  Vec2 spriteSize;
  Vec2 pos;
  Vec2 size;
};

struct GlobalData
{
  float gameTime;
  float padding;
  IVec2 windowSize;
  Mat4 orthProjGame;
};

struct RenderData
{
  OrthographicCamera2D gameCam;
  OrthographicCamera2D lightsCam;
  GlobalData globalData;

  int transformCount = 0;
  Transform transforms[MAX_TRANSFORMS];

  int lightCount = 0;
  Transform lights[MAX_TRANSFORMS];
};

struct TextData
{


};


// #############################################################################
//                           Render Globals
// #############################################################################
static RenderData renderData = {};

// #############################################################################
//                           Render Helper Functions
// #############################################################################

void precise_wait_seconds(double seconds)
{
  // if our machine can produce 70 fps maximum, but we limit 120
  if(seconds < 0.0) return;

  u64 start_tick = SDL_GetPerformanceCounter();
  u64 frequency = SDL_GetPerformanceFrequency();
  u64 target_tick = start_tick + static_cast<u64>(seconds * static_cast<double>(frequency)); // don't complain about the cast, msvc warnings sir

  while (SDL_GetPerformanceCounter() < target_tick)
  {
    SDL_CPUPauseInstruction();
    // _mm_pause();
  }
}


// float world_to_ui_x(float x)
// {
//   const OrthographicCamera2D& gameCam = renderData->gameCamera;
//   const OrthographicCamera2D& uiCam = renderData->uiCamera;
//   float xPos = (x) / gameCam.dimensions.x * uiCam.dimensions.x; // [0; dimensions.x]
//   return xPos;
// }
// float world_to_ui_y(float y)
// {
//   const OrthographicCamera2D& gameCam = renderData->gameCamera;
//   const OrthographicCamera2D& uiCam = renderData->uiCamera;
//   float yPos = (y) / gameCam.dimensions.y * uiCam.dimensions.y; // [0; dimensions.y]
//   return yPos;
// }

// Vec2 world_to_ui(Vec2 worldPos)
// {
//   const OrthographicCamera2D& gameCam = renderData->gameCamera;
//   const OrthographicCamera2D& uiCam = renderData->uiCamera;

//   float xPos = world_to_ui_x(worldPos.x);
//   float yPos = world_to_ui_y(worldPos.y);

//   return {xPos, yPos};
// }

// #############################################################################
//                           Render Functions
// #############################################################################
void draw_sprite(const SpriteID spriteID, Vec2 pos, float scale = 1)
{
  const Sprite& sprite = SPRITES[spriteID];
  Transform t = 
  {
    .atlasPos = vec_2(sprite.atlasOffset),
    .spriteSize = vec_2(sprite.size),
    .pos = pos - vec_2(sprite.size/2 - sprite.pivotOffset) * scale,
    .size = vec_2(sprite.size) * scale,
  };
  renderData.transforms[renderData.transformCount++] = t;
}

void draw_light(Vec2 pos, float scale = 1)
{
  const Sprite& sprite = SPRITES[SPRITE_LIGHT];
  Transform t = 
  {
    .atlasPos = vec_2(sprite.atlasOffset),
    .spriteSize = vec_2(sprite.size),
    .pos = pos - vec_2(sprite.size/2 - sprite.pivotOffset) * scale,
    .size = vec_2(sprite.size) * scale,
  };
  renderData.lights[renderData.lightCount++] = t;
}

void draw_ui_text(char* text, Vec2 pos)
{
  // TODO: Implement
}