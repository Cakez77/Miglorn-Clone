#pragma once
#include "lib.h"
#include "assets.h"
#include "raylib.h"

static Texture atlas;

void draw_sprite(const SpriteID spriteID, Vec2 pos)
{
  const Sprite& sprite = SPRITES[spriteID];
  Rectangle rectangle = {
    sprite.atlasOffset.x,
    sprite.atlasOffset.y, 
    sprite.size.x, sprite.size.y};
  
  pos.x += -sprite.size.x/2 - sprite.pivotOffset.x;
  pos.y += -sprite.size.y/2 - sprite.pivotOffset.y;

  DrawTextureRec(atlas, rectangle, pos, WHITE);
}