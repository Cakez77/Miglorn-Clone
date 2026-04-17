#pragma once
#include "lib.h"


struct Sprite2
{
	IVec2 atlasOffset;
	IVec2 size;
	IVec2 pivotOffset;
	int frameCount;
};

enum SpriteID2 
{
	SPRITE_BUTTON,
	SPRITE_KNIGHT_IDLE,
	SPRITE_KNIGHT_ATTACK,
	SPRITE_PINE_TREE,
	SPRITE_COUNT2
};

Sprite2 SPRITES2[SPRITE_COUNT2] =
{
	[SPRITE_BUTTON] = {.atlasOffset={336, 0}, .size={64, 26}, .pivotOffset={0, 0}, .frameCount=1},
	[SPRITE_KNIGHT_IDLE] = {.atlasOffset={0, 0}, .size={48, 48}, .pivotOffset={1, -11}, .frameCount=2},
	[SPRITE_KNIGHT_ATTACK] = {.atlasOffset={0, 0}, .size={48, 48}, .pivotOffset={1, -11}, .frameCount=6},
	[SPRITE_PINE_TREE] = {.atlasOffset={288, 0}, .size={48, 64}, .pivotOffset={0, -22}, .frameCount=1},

};

