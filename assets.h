#pragma once
#include "lib.h"

enum SpriteID
{
    SPRITE_PLAYER,
    SPRITE_VIKING_HOUSE,
    SPRITE_PINE_TREE,
    SPRITE_BONFIRE,
    SPRITE_EMBER,

    SPRITE_COUNT
};

struct Sprite
{
    Vector2 atlasOffset;
    Vector2 size;
    Vector2 pivotOffset;
    int frameCount;
};

const Sprite SPRITES[SPRITE_COUNT] = 
{
  [SPRITE_PLAYER] = {.atlasOffset = {0, 0}, .size = {48, 48}, .pivotOffset = {0, -10}},
  [SPRITE_VIKING_HOUSE] = {.atlasOffset = {288, 0}, .size = {80, 96}, .pivotOffset = {0, -16}},
  [SPRITE_PINE_TREE] = {.atlasOffset = {368, 0}, .size = {48, 64}, .pivotOffset = {0, -24}},
  [SPRITE_BONFIRE] = {.atlasOffset = {0, 96}, .size = {48, 16}},
  [SPRITE_EMBER] = {.atlasOffset = {64, 96}, .size = {10, 10}},
};