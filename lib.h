#pragma once
#define RAYMATH_IMPLEMENTATION
#include "raylib.h"
#include "raymath.h"

typedef Vector2 Vec2;
const Vec2 VEC2_ZERO = {0, 0};

float min(float a, float b)
{
  return (a < b)? a : b;
}

float max(float a, float b)
{
  return (a > b)? a : b;
}

float approach(float current, float target, float increase)
{
  // Acceleration
  if(current < target)
  {
    return min(current + increase, target);
  }

  return max(current - increase, target);
}