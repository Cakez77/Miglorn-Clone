#ifdef ENGINE // Inside game / engine
#pragma  once
#include "lib.h"

#define vec2 Vec2
#define ivec2 IVec2
#define vec3 Vec3
#define vec4 Vec4
#define uint u32
#else // Inside Shader

#ifdef HLSL // DirectX
#else // OpenGL
#endif

#define BIT(i) 1 << i

#endif

struct Color
{
  #ifdef ENGINE
  union
  {
    u32 hex;
    unsigned char channels[4];
  };

  bool operator==(const Color& other)
  {
    return hex == other.hex;
  }
  bool operator!=(const Color& other)
  {
    return !(hex == other.hex);
  }
  #else  // in the shader, just int's
  uint hex;
  #endif
};

// #############################################################################
//                           Shader Constants
// #############################################################################
// Max 24 Bits renderOptions!!!
const int RENDER_OPTION_LIGHT_RAYMARCH = BIT(0);
const int RENDER_OPTION_LINEAR_FILTERING = BIT(1);

// #############################################################################
//                           Shader Structs
// #############################################################################
struct Material
{
  Color color;
  Color add;

  #ifdef ENGINE
  bool operator==(const Material other)
  {
    return color == other.color && add == other.add;
  }
  #endif
};

struct Transform
{
  int atlasPosPacked;
  int spriteSize;
  vec2 pos;
  vec2 size;
  int pack1; // [renderOptions - 24 Bits][fontIdx - 4 Bits][matrixIdx - 4 Bits]
  float layer;
  uint color;
  uint addColor;
  // int padding[2];
};

vec4 get_color(uint hex)
{
  vec4 color;
  // RGBA if using hex(uint) on the CPU
  color.r = float((hex >> 24) & uint(0xFF)) / 255.0;
  color.g = float((hex >> 16) & uint(0xFF)) / 255.0;
  color.b = float((hex >>  8) & uint(0xFF)) / 255.0;
  color.a = float((hex)       & uint(0xFF)) / 255.0;

  // ABGR, if using u8 channels[4] on the CPU
  // color.a = float((hex >> 24) & uint(0xFF)) / 255.0;
  // color.b = float((hex >> 16) & uint(0xFF)) / 255.0;
  // color.g = float((hex >>  8) & uint(0xFF)) / 255.0;
  // color.r = float((hex)       & uint(0xFF)) / 255.0;
  return color;
}

#ifndef ENGINE
float gradientNoise(vec2 uv) 
{
  const vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
  return fract(magic.z * fract(dot(uv, magic.xy)));
}
#endif