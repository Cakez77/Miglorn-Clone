#pragma once
#include "lib.h"


struct Sprite
{
	Vec2 atlasOffset;
	Vec2 size;
	Vec2 pivotOffset;
	int frameCount;
};

enum SpriteID 
{
	SPRITE_BONFIRE,
	SPRITE_BUTTON,
	SPRITE_EMBER,
	SPRITE_KNIGHT_IDLE,
	SPRITE_KNIGHT_ATTACK,
	SPRITE_LIGHT_32,
	SPRITE_PINE_TREE,
	SPRITE_SDV,
	SPRITE_TORCH,
	SPRITE_UI_SLICE_WOOD_01,
	SPRITE_VIKING_HOUSE,
	SPRITE_WHITE,
	SPRITE_WOODEN_STAKES,
	SPRITE_COUNT2
};

const Sprite SPRITES[SPRITE_COUNT] =
{
	[SPRITE_BONFIRE] = {.atlasOffset={1024, 0}, .size={48, 16}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_BUTTON] = {.atlasOffset={960, 0}, .size={64, 26}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_EMBER] = {.atlasOffset={1088, 0}, .size={10, 10}, .pivotOffset={0, 0}, .frameCount=4},
	[SPRITE_KNIGHT_IDLE] = {.atlasOffset={480, 0}, .size={48, 48}, .pivotOffset={1, -11}, .frameCount=2},
	[SPRITE_KNIGHT_ATTACK] = {.atlasOffset={480, 0}, .size={48, 48}, .pivotOffset={1, -11}, .frameCount=6},
	[SPRITE_LIGHT_32] = {.atlasOffset={912, 0}, .size={32, 32}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_PINE_TREE] = {.atlasOffset={848, 0}, .size={48, 64}, .pivotOffset={0, -22}, .frameCount=1},
	[SPRITE_SDV] = {.atlasOffset={0, 0}, .size={480, 272}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_TORCH] = {.atlasOffset={944, 0}, .size={16, 32}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_UI_SLICE_WOOD_01] = {.atlasOffset={1072, 0}, .size={16, 16}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_VIKING_HOUSE] = {.atlasOffset={768, 0}, .size={80, 96}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_WHITE] = {.atlasOffset={1128, 0}, .size={1, 1}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_WOODEN_STAKES] = {.atlasOffset={896, 0}, .size={16, 40}, .pivotOffset={0, -13}, .frameCount=1},

};

