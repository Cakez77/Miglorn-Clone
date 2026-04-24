#pragma once
#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef WEB_BUILD
// the web build does not inlclude math stuff
#include <math.h>
#include <cstdlib>
#endif

// #############################################################################
//                           Defines
// #############################################################################
#define PI SDL_PI_F
#define INVALID_IDX -1

#define line_id(index) (size_t)((__LINE__ << 8) | (index))

#define u8 uint8_t
#define s8 int8_t
#define u16 uint16_t
#define s16 int16_t
#define u32 uint32_t
#define i32 int32_t
#define u64 uint64_t
#define s64 int64_t

#define BIT(x) (1u << (x))
#define BIT64(x) (1ull << (x))
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))

#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))

// Web builds don't export functions, we include game.cpp
// instead of loading the game.dll
#ifdef WEB_BUILD
#define EXPORT_FN
#else
#define EXPORT_FN __declspec(dllexport)
#endif

// #############################################################################
//                           Assertions
// #############################################################################
// The ## in front __VA_ARGS__ is so that sometimes you don't supply 
// the arguments, it will "just work"
#define ASSERT_MSG(x, msg, ...)                                 \
if(!(x))                                                        \
{                                                               \
  SDL_LogCritical(SDL_LOG_CATEGORY_ASSERT, msg, ##__VA_ARGS__); \
  SDL_assert((x));                                              \
}

// #############################################################################
//                           String Stuff
// #############################################################################
template <typename... Args>
char* format_text(const char* format, Args... args)
{
  static int bufferIdx = 0;
  static char buffers[8][4096] = {};
  
  char* buffer = buffers[bufferIdx];
  bufferIdx = (bufferIdx + 1) % 4;
  memset(buffer, 0, 1024);
  
  SDL_snprintf(buffer, 4096, format, args...);
  
  return buffer;
}

int str_len(const char *string)
{
  int strLength = 0;
  if (string)
  {
    while (*(string++))
    {
      strLength++;
    }
  }  
  return strLength;
}

bool str_cmp(char *a, char *b)
{
  return a && b ? 
         str_len(a) == str_len(b) ? 
         strncmp(a, b, str_len(a)) == 0 ? 
         true : false : false : false;
}

unsigned int decode_utf8(const char8_t** p)
{
  unsigned c = *(*p)++; 
  if (c < 0x80)
  {
    return c; 
  }

  if (c < 0xE0)
  {
    return ((c&0x1F)<<6) | (*(*p)++&0x3F); 
  }

  if (c < 0xF0)
  {
    return ((c&0x0F)<<12) | ((*(*p)++&0x3F)<<6) | (*(*p)++&0x3F); 
  }
    
  return ((c&0x07)<<18) | ((*(*p)++&0x3F)<<12) | ((*(*p)++&0x3F)<<6) | (*(*p)++&0x3F); 
}

struct Words
{
  int count;
  unsigned int values[512][128] = {};
};

Words split_words(char* text)
{
  Words words = {};
  int charIdx = 0;

  while(unsigned int codePoint = decode_utf8((const char8_t**)&text))
  {
    if(codePoint == ' ' && charIdx > 0)
    {
      words.count++;
      charIdx = 0;
      continue;
    }
    
    
    if(codePoint == '\n')
    {
      if(charIdx > 0)
      {
        charIdx = 0;
        words.values[++words.count][charIdx] = codePoint;
        words.count++;
      }
      else
      {
        words.values[words.count++][charIdx] = codePoint;
      }
      
      continue;
    }

    if(codePoint == '[' && *text == 'h')
    {
      words.values[words.count][charIdx++] = codePoint;
      while(unsigned int codePoint = decode_utf8((const char8_t**)&text))
      {
        words.values[words.count][charIdx++] = codePoint;
        if(codePoint == ']')
        break;
      }
      words.count++;
      charIdx = 0;
      continue;
    }
    
    words.values[words.count][charIdx++] = codePoint;
  }
  
  unsigned int* lastWord = words.values[words.count];
  if(*lastWord && *lastWord != '\n')
  {
    words.count++;
  }
  
  return words;
}

// #############################################################################
//                           Easing Functions
// #############################################################################
float ease_out_slowdown(float t)
{
  if(t < 1.0f)
  {
    return 1 - expf(-4*t);
  }
  else
  {
    return 1;
  }
}

float ease_out_linear(float t)
{
  if(t < 0.0f)
  {
    return 0.0f;
  }
  else if(t < 1.0f)
  {
    return t;
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_quad(float t)
{
  if (t < 1.0f)
  {
    return t * t;
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_quad(float t)
{
  if (t < 1.0f && t >= 0.0f)
  {
    return 1.0f - (1.0f - t) * (1.0f - t);
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_qubic(float t)
{
  if (t < 1.0f)
  {
    return t * t * t * t;
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_qubic(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t) * (1.0f - t);
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_out_qubic(float t)
{
  if (t < 1.0f)
  {
    return t < 0.5f ? 4.0f * t * t * t : 1 - (float)pow(-2 * t + 2, 3) / 2.0f;
  }
  else
  {
    return 1.0f;
  }
}

float ease_wind_slash(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - (float)pow(-2 * (t) + 2, 5) / 33.0f;
  }
  else
  {
    return 1.0f;
  }
}

float ease_arrow(float t)
{
  if (t < 1.0f)
  {
    return t <= 0.3f ? 16.0f * t * t * t : 1 - (float)pow(-2 * (t + 0.111) + 2, 5) / 4.0f;
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_expo(float t)
{
  if (t < 1.0f)
  {
    return (float)pow(2, 8 * t - 8);
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_expo(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - (float)pow(2, -10 * t);
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_quint(float t)
{
  if(t < 1.0f)
  {
    return 1.0f - powf(1.0f - t, 5.0f);
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_circ(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - sqrtf(1 - t * t);
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_elastic(float t)
{
  float c4 = (2.0f * 3.14f) / 3.0f;
  
  if (t == 0.0f)
  {
    return 0.0f;
  }
  else if (t < 1.0f)
  {
    return (float)pow(2, -10 * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_back(float t)
{
  if(t < 0)
  {
    return 0;
  }
  float c1 = 1.70158f;
  float c3 = c1 + 1.0f;
  if (t < 1.0f)
  {
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
  }
  else
  {
    return 1.0f;
  }
}

// #############################################################################
//                           Math stuff
// #############################################################################
float min(float a, float b)
{
  return (a < b)? a : b;
}

float max(float a, float b)
{
  return (a > b)? a : b;
}

float clamp(float x, float min, float max)
{
  if(x < min)
  {
    return min;
  }

  if(x > max)
  {
    return max;
  }

  return x;
}

int min(int a, int b)
{
  return (a < b)? a : b;
}

int max(int a, int b)
{
  return (a > b)? a : b;
}

int clamp(int x, int min, int max)
{
  if(x < min)
  {
    return min;
  }

  if(x > max)
  {
    return max;
  }

  return x;
}

short max(short a, short b)
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

int sign(int x)
{
  return (x >= 0)? 1 : -1;
}

float sign(float x)
{
  return (x >= 0.0f)? 1.0f : -1.0f;
}

float random_range(float min, float max)
{
  // [0;1]
  float randomFloat = (float)rand() / (float)RAND_MAX;
  
  return (max - min) * randomFloat + min;
}

int random_range_int(int min, int max)
{
  // [0;1]
  float randomFloat = (float)rand() / (float)RAND_MAX;
  
  return (int)((max - min) * randomFloat + min);
}

// ################################################
//          Vector 2
// ################################################
struct Vec2 
{
  float x;
  float y;
  operator bool() const {return x != 0.0f || y != 0.0f;}
};
const Vec2 VEC2_ZERO = Vec2{0.0f, 0.0f};

Vec2 random_dir()
{
  const float randomAngle = random_range(0, 2 * PI);
  Vec2 dir = Vec2{sinf(randomAngle), cosf(randomAngle)};

  return dir;
}

Vec2 operator+(const Vec2& a, const Vec2& b)
{
  return Vec2{a.x + b.x, a.y + b.y};
}

Vec2& operator+=(Vec2& a, Vec2 b)
{
  a = a + b;
  return a;
}

Vec2 operator+(Vec2 a, float scalar)
{
  return Vec2{a.x + scalar, a.y + scalar};
}

Vec2& operator+=(Vec2& a, float scalar)
{
  a = a + scalar;
  return a;
}

Vec2 operator+(Vec2 a, int scalar)
{
  return Vec2{a.x + scalar, a.y + scalar};
}

Vec2 operator*(Vec2 a, Vec2 b)
{
  return Vec2{a.x * b.x, a.y * b.y};
}

Vec2& operator*=(Vec2& a, Vec2 b)
{
  a = a * b;
  return a;
}

Vec2 operator*(Vec2 a, float scalar)
{
  return Vec2{a.x * scalar, a.y * scalar};
}

Vec2 operator*(Vec2 a, int scalar)
{
  return Vec2{a.x * scalar, a.y * scalar};
}

Vec2& operator*=(Vec2& a, float scalar)
{
  a = a * scalar;
  return a;
}

constexpr Vec2 operator/(Vec2 a, int scalar) 
{
  return Vec2{a.x / (float)scalar, 
              a.y / (float)scalar};
}

Vec2 operator/(Vec2 a, float scalar)
{
  return Vec2{a.x / scalar, a.y / scalar};
}

Vec2 operator-(Vec2 a)
{
  return Vec2{-a.x, -a.y};
}

Vec2 operator-(Vec2 a, Vec2 b)
{
  return Vec2{a.x - b.x, a.y - b.y};
}

Vec2& operator-=(Vec2& a, Vec2 b)
{
  a = a - b;
  return a;
}

Vec2 operator-(Vec2 a, float scalar)
{
  return Vec2{a.x - scalar, a.y - scalar};
}

Vec2 operator-(Vec2 a, int scalar)
{
  return Vec2{a.x - scalar, a.y - scalar};
}

Vec2& operator-=(Vec2& a, float scalar)
{
  a = a - scalar;
  return a;
}

bool operator==(Vec2 a, Vec2 b)
{
  return a.x == b.x && a.y == b.y;
}

bool operator!=(Vec2 a, Vec2 b)
{
  return !(a == b);
}

struct IVec2
{
  int x;
  int y;

  operator bool() {return x != 0 || y != 0;}
};
constexpr IVec2 IVEC2_ZERO = {0, 0};

bool operator==(IVec2 a, IVec2 b)
{
  return a.x == b.x && a.y == b.y;
}

bool operator!=(IVec2 a, IVec2 b)
{
  return !(a == b);
}

constexpr IVec2 operator/(IVec2 a, float scalar)
{
  return IVec2{(int)((float)a.x / scalar), 
               (int)((float)a.y / scalar)};
}

IVec2 operator/(IVec2 a, IVec2 b)
{
  return IVec2{a.x / b.x, a.y / b.y};
}

IVec2 operator-(IVec2 a, IVec2 b)
{
  return IVec2{a.x - b.x, a.y - b.y};
}

Vec2 operator-(IVec2 a, Vec2 b)
{
  return Vec2{a.x - b.x, a.y - b.y};
}

IVec2 operator-(IVec2 a, int scalar)
{
  return IVec2{a.x - scalar, a.y - scalar};
}

IVec2& operator-=(IVec2& a, IVec2 b)
{
  a = a - b;
  return a;
}

IVec2 operator+(IVec2 a, IVec2 b)
{
  return IVec2{a.x + b.x, a.y + b.y};
}

IVec2 operator+(IVec2 a, Vec2 b)
{
  return IVec2{a.x + (int)b.x, a.y + (int)b.y};
}

IVec2& operator+=(IVec2& a, IVec2 b)
{
  a = a + b;
  return a;
}

IVec2 operator+(IVec2 a, int scalar)
{
  return IVec2{a.x + scalar, a.y + scalar};
}

IVec2& operator+=(IVec2& a, Vec2 other)
{
  a = a + other;
  return a;
}

IVec2& operator+=(IVec2& a, int scalar)
{
  a = a + scalar;
  return a;
}

IVec2 operator*(IVec2 a, int scalar)
{
  return IVec2{a.x * scalar, a.y * scalar};
}

IVec2 operator*(IVec2 a, float scalar)
{
  return IVec2{(int)((float)a.x * scalar), (int)((float)a.y * scalar)};
}

IVec2& operator*=(IVec2& a, int scalar)
{
  a = a * scalar;
  return a;
}

Vec2 max_vec_2(Vec2 v, const Vec2& max)
{
  if(v.x > max.x || v.y > max.y)
  {
    if(v.x > v.y)
    {
      const float scale = max.x / v.x;
      v *= scale;
    }
    else
    {
      const float scale = max.y / v.y;
      v *= scale;
    }
  }
  return v;
}

Vec2 clamp_vec_2(Vec2 v, const Vec2&min, const Vec2& max)
{
  v.x = clamp(v.x, min.x, max.x);
  v.y = clamp(v.y, min.y, max.y);
  return v;
}

Vec2 vec_2(IVec2 ivec2)
{
  return Vec2{(float)ivec2.x, (float)ivec2.y};
}

Vec2 vec_2(float scalar)
{
  return Vec2{scalar, scalar};
}

IVec2 ivec_2(int scalar)
{
  return IVec2{scalar, scalar};
}

IVec2 ivec_2(float scalar)
{
  return IVec2{(int)scalar, (int)scalar};
}

IVec2 ivec_2(Vec2 v)
{
  return IVec2{(int)v.x, (int)v.y};
}

Vec2 min_vec2(Vec2 a, Vec2 b)
{
  return {min(a.x, b.x), min(a.y, b.y)};
}

Vec2 max_vec2(Vec2 a, Vec2 b)
{
  return {max(a.x, b.x), max(a.y, b.y)};
}

IVec2 min_ivec2(IVec2 a, IVec2 b)
{
  return {min(a.x, b.x), min(a.y, b.y)};
}

IVec2 max_ivec2(IVec2 a, IVec2 b)
{
  return {max(a.x, b.x), max(a.y, b.y)};
}

Vec2 floor_vec2(Vec2 v)
{
  return Vec2{floorf(v.x), floorf(v.y)};
}

float lerp(float a, float b, float t)
{
  if(t > 1.0f)
  {
    t = 1.0f;
  }

  return a + (b - a) * t;
}

float lerp_no_cap(float a, float b, float t)
{
  float result = a * (1.0f - t) + b * t;
  return result;
}

float length(Vec2 v)
{
  return sqrtf(v.x * v.x + v.y * v.y);
}

float length(Vec2 a, Vec2 b)
{
  return length(b - a);
}

Vec2 random_point_in_circle(Vec2 center, float radius)
{
  float randomAngle = random_range(0, 2*PI);
  float randomRange = random_range(0, radius);
  Vec2 pos = center + Vec2{sinf(randomAngle), cosf(randomAngle)} * randomRange;
  return pos;
};

Vec2 normalize(Vec2 v)
{
  Vec2 normalized = {};
  float vecLength = length(v);
  if(vecLength)
  {
    normalized = v / vecLength;
  }
  else
  {
    // SM_ASSERT(0, "Vector has a length of 0");
  }

  return normalized;
}

Vec2 lerp(Vec2 p0, Vec2 p1, float t)
{
  t = clamp(t, 0.0f, 1.0f); 
  Vec2 result = p0 * (1.0f - t) + p1 * t;
  return result;
}

Vec2 lerp_no_cap(Vec2 p0, Vec2 p1, float t)
{
  Vec2 result = p0 * (1.0f - t) + p1 * t;
  return result;
}

IVec2 lerp(IVec2 a, IVec2 b, float t)
{
  IVec2 result;
  result.x = (int)floorf(lerp((float)a.x, (float)b.x, t));
  result.y = (int)floorf(lerp((float)a.y, (float)b.y, t));
  return result;
}

Vec2 rotate_vec2(Vec2 pos, float angle)
{
  float newX = pos.x * cosf(angle) - pos.y * sinf(angle);
  float newY = pos.x* sinf(angle) + pos.y * cosf(angle);
  
  return {newX, newY};
}

Vec2 quadratic_bezier(Vec2 p0, Vec2 p1, Vec2 m, float t)
{
  Vec2 p0ToMiddle = lerp(p0, m, t);
  Vec2 middleToP1 = lerp(m, p1, t);
  Vec2 middlePoint = lerp(p0ToMiddle, middleToP1, t);
  
  return middlePoint;
}

Vec2 bezier_over_time(Vec2 start, Vec2 end, float t, float middleDivide = 2.0f, float height = 160.0f)
{
  const Vec2 middle = start + (end - start) / middleDivide + Vec2{0, - height};
  Vec2 samplePoint = quadratic_bezier(start, end, middle, t);
  return samplePoint;
}

Vec2 cubic_bezier(Vec2 p0, Vec2 m0, Vec2 p1, Vec2 m1, float t)
{
  Vec2 p0ToM0 = lerp(p0, m0, t);
  Vec2 m0ToM1 = lerp(m0, m1, t);
  Vec2 m1ToP1 = lerp(m1, p1, t);
  
  Vec2 p0ToM0_to_m0ToM1 = lerp(p0ToM0, m0ToM1, t);
  Vec2 m0ToM1_to_m1ToP1 = lerp(m0ToM1, m1ToP1, t);
  
  Vec2 middlePoint = lerp(p0ToM0_to_m0ToM1, m0ToM1_to_m1ToP1, t);
  
  return middlePoint;
}

float length_squared(Vec2 v)
{
  return (v.x * v.x) + (v.y * v.y);
}

float length_squared(IVec2 v)
{
  return length_squared(vec_2(v));
}

Vec2 normal_from_vec_top(Vec2 v)
{
  return {v.y, -v.x};
}

Vec2 normal_from_vec_bottom(Vec2 v)
{
  return {v.y, v.x};
}

float dot(Vec2 a, Vec2 b)
{
  return a.x * b.x + a.y * b.y;
}

Vec2 reflect_by_normal(Vec2 normal, Vec2 direction)
{
  //Taken from https://math.stackexchange.com/questions/13261/how-to-get-a-reflection-vector
  normal = normalize(normal);
  return direction - normal * 2.0f * dot(direction, normal);
}

Vec2 rotate_by_direction(Vec2 point, Vec2 d)
{
  float angle = atan2f(d.y, d.x) + 3.14f / 2.0f;
  
  return {(point.x) * sinf(angle) - (point.y) * cosf(angle),
    (point.x) * cosf(angle) + (point.y) * sinf(angle)};
}

Vec2 sin_between_two_points(Vec2 a, Vec2 b, float t, float speedup = 1.0f)
{
  Vec2 direction = b - a;
  
  // [0:1] range
  float progress = sinf(t * speedup);
  
  Vec2 pos = a + direction * progress;
  return pos;
}

float get_angle(Vec2 direction)
{
  return (atan2f(direction.y, direction.x) + PI / 2.0f);
}

float get_angle_correct(Vec2 direction)
{
  return atan2f(direction.y, direction.x);
}

float get_angle_degress(float deg)
{
  int i = (int)(deg / 360.0f);
  deg -= (float)i * 360.0f;
  
  if(deg > 180)
  {
    deg -= 360.0f;
  }
  else if(deg < -180)
  {
    deg += 360.0f;
  }
  
  return deg;
}

float get_angle_radians(float rad)
{
  float tau = 6.28f;
  int i = (int)(rad / tau);
  
  rad -= (float)i * tau;
  if(rad > 3.14f) // Kinda Modulo
  {
    rad -= tau;
  }
  else if(rad < -3.14f)
  {
    rad += tau;
  }
  
  return rad;
}

// #############################################################################
//                                Vector 4
// #############################################################################
struct Vec4
{
  union
  {
    float values[4];
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };
    
    struct
    {
      float r;
      float g;
      float b;
      float a;
    };
    
    struct
    {
      float values[2];
    }xy;
    
    struct
    {
      float values[3];
    }xyz;
  };

  operator bool()
  {
    return a != 0.0f || r != 0.0f || g != 0.0f || b != 0.0f;
  }
  
  bool operator==(const Vec4& other) const
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }
  
  Vec4 operator+(const Vec4& other) const
  {
    return {x + other.x,
      y + other.y,
      z + other.z,
      w + other.w};
  }

  Vec4& operator+=(const Vec4& other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    
    return *this;
  }
  
  Vec4 operator+(float value) const
  {
    return {x + value,
      y + value,
      z + value,
      w + value};
  }
  
  Vec4 operator*(float value) const
  {
    return {x * value,
      y * value,
      z * value,
      w * value};
  }

  Vec4 operator/(float value) const
  {
    return {x / value,
      y / value,
      z / value,
      w / value};
  }

  Vec4 operator/(const Vec4& other) const
  {
    return {x / other.x,
      y / other.y,
      z / other.z,
      w / other.w};
  }
  
  Vec4 operator-(float value) const
  {
    return {x - value,
      y - value,
      z - value,
      w - value};
  }

  Vec4 operator-(const Vec4& other) const
  {
    return {x - other.x,
      y - other.y,
      z - other.z,
      w - other.w};
  }
  
  Vec4 operator*(const Vec4& other) const
  {
    return {x * other.x,
      y * other.y,
      z * other.z,
      w * other.w};
  }
  
  Vec4& operator*=(float value)
  {
    x *= value;
    y *= value;
    z *= value;
    w *= value;
    
    return *this;
  }
  
  Vec4& operator*=(const Vec4& other)
  {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;
    
    return *this;
  }

  float& operator[](int index)
  {
    return values[index];
  }
};

Vec4 vec_4(float scalar)
{
  return Vec4{scalar, scalar, scalar, scalar};
}

Vec4 lerp(const Vec4& a, const Vec4& b, float t)
{
  if(t > 1.0f)
  {
    t = 1.0f;
  }
  
  Vec4 result = a * (1.0f - t) + b * t;
  return result;
}

Vec4 floorV4(const Vec4& v)
{
  return {floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w)};
}

struct IVec4
{
  union
  {
    int values[4];
    struct
    {
      int x;
      int y;
      int z;
      int w;
    };
    union
    {
      struct
      {
        int xy[2];
      };
      struct
      {
        int padding[2];
        int zw[2];
      };
    };
    
    struct
    {
      int xyz[3];
    };
  };
};

// #############################################################################
//                              Rects
// #############################################################################
struct Rect
{
  union
  {
    Vec2 offset;
    Vec2 pos;
  };
  Vec2 size;
  float angle;
};

bool operator !=(const Rect& a, const Rect& b)
{
  return a.pos != b.pos || a.size != b.size || a.angle != b.angle;
}

const Rect RECT_ZERO = {{0, 0}, {0, 0}, 0};

bool point_in_rect(Vec2 point, Rect rect)
{
  return (point.x >= rect.pos.x &&
          point.x <= rect.pos.x + rect.size.x &&
          point.y >= rect.pos.y &&
          point.y <= rect.pos.y + rect.size.y);
}

bool point_in_rect_center(Vec2 point, Rect rect)
{
  return (point.x >= rect.pos.x - rect.size.x / 2 &&
          point.x <= rect.pos.x + rect.size.x / 2 &&
          point.y >= rect.pos.y - rect.size.y / 2 &&
          point.y <= rect.pos.y + rect.size.y / 2);
}

bool point_in_rect(Vec2 point, Rect rect, float angle)
{
  Vec2 boxMiddle = rect.pos + rect.size / 2.0f;
  Vec2 boxUp = {0.0f, 1.0f};
  boxUp = rotate_vec2(boxUp, angle);
  Vec2 boxRight = normal_from_vec_top(boxUp);
  Vec2 boxToPointDir = point - boxMiddle;
  
  float upProjection = dot(boxToPointDir, boxUp);
  float rightProjection = dot(boxToPointDir, boxRight);
  
  float halfWidth = rect.size.x / 2.0f;
  float halfHeight = rect.size.y / 2.0f;
  
  return 
  (upProjection >= -halfHeight && upProjection <= halfHeight) &&
  (rightProjection >= -halfWidth && rightProjection <= halfWidth);
}

inline void AddVectors2D(Vec2 * v1, Vec2 * v2)
{ v1->x += v2->x; v1->y += v2->y; }

inline void SubVectors2D(Vec2 * v1, Vec2 * v2)
{ v1->x -= v2->x; v1->y -= v2->y; }

inline void rotate_vec2_clockwise(Vec2 * v, float ang)
{
  float t, cosa = cos(ang), sina = sin(ang);
  t = v->x; 
  v->x = t*cosa + v->y*sina; 
  v->y = -t*sina + v->y*cosa;
}

// Rotated Rectangles Collision Detection, Oren Becker, 2001
int rot_rect_collision(Rect * rr1, Rect * rr2)
{
  Vec2 A, B,  // vertices of the rotated rr2
	  C,        // center of rr2
	  BL, TR;   // vertices of rr2 (bottom-left, top-right)

  float ang = rr1->angle - rr2->angle,  // orientation of rotated rr1
    cosa = cos(ang),                    // precalculated trigonometic -
    sina = sin(ang);                    // - values for repeated use

  float t, x, a;      // temporary variables for various uses
  float dx;           // deltaX for linear equations
  float ext1, ext2;   // min/max vertical values

  // move rr2 to make rr1 cannonic
  C = rr2->pos;
  SubVectors2D(&C, &rr1->pos);

  // rotate rr2 clockwise by rr2->ang to make rr2 axis-aligned
  rotate_vec2_clockwise(&C, rr2->angle);

  // calculate vertices of (moved and axis-aligned := 'ma') rr2
  BL = TR = C;
  Vec2 rr1Size = rr1->size * 0.5f;
  Vec2 rr2Size = rr2->size * 0.5f;
  SubVectors2D(&BL, &rr2Size);
  AddVectors2D(&TR, &rr2Size);

  // calculate vertices of (rotated := 'r') rr1
  A.x = -rr1Size.y*sina; B.x = A.x; t = rr1Size.x*cosa; A.x += t; B.x -= t;
  A.y =  rr1Size.y*cosa; B.y = A.y; t = rr1Size.x*sina; A.y += t; B.y -= t;

  t = sina*cosa;

  // verify that A is vertical min/max, B is horizontal min/max
  if (t < 0)
  {
    t = A.x; A.x = B.x; B.x = t;
    t = A.y; A.y = B.y; B.y = t;
  }

  // verify that B is horizontal minimum (leftest-vertex)
  if (sina < 0) { B.x = -B.x; B.y = -B.y; }

  // if rr2(ma) isn't in the horizontal range of
  // colliding with rr1(r), collision is impossible
  if (B.x > TR.x || B.x > -BL.x) return 0;

  // if rr1(r) is axis-aligned, vertical min/max are easy to get
  if (t == 0) {ext1 = A.y; ext2 = -ext1; }
  // else, find vertical min/max in the range [BL.x, TR.x]
  else
  {
    x = BL.x-A.x; a = TR.x-A.x;
    ext1 = A.y;
    // if the first vertical min/max isn't in (BL.x, TR.x), then
    // find the vertical min/max on BL.x or on TR.x
    if (a*x > 0)
    {
      dx = A.x;
      if (x < 0) { dx -= B.x; ext1 -= B.y; x = a; }
      else       { dx += B.x; ext1 += B.y; }
      ext1 *= x; ext1 /= dx; ext1 += A.y;
    }
  
    x = BL.x+A.x; a = TR.x+A.x;
    ext2 = -A.y;
    // if the second vertical min/max isn't in (BL.x, TR.x), then
    // find the local vertical min/max on BL.x or on TR.x
    if (a*x > 0)
    {
    dx = -A.x;
    if (x < 0) { dx -= B.x; ext2 -= B.y; x = a; }
    else       { dx += B.x; ext2 += B.y; }
    ext2 *= x; ext2 /= dx; ext2 -= A.y;
    }
  }

  // check whether rr2(ma) is in the vertical range of colliding with rr1(r)
  // (for the horizontal range of rr2)
  return !((ext1 < BL.y && ext2 < BL.y) || (ext1 > TR.y && ext2 > TR.y));
}

// #############################################################################
//                           Matrix 4
// #############################################################################
struct Mat4
{
  union 
  {
    Vec4 values[4];
    struct
    {
      float ax;
      float bx;
      float cx;
      float dx;

      float ay;
      float by;
      float cy;
      float dy;

      float az;
      float bz;
      float cz;
      float dz;
      
      float aw;
      float bw;
      float cw;
      float dw;
    };
  };

  Vec4& operator[](int col)
  {
    return values[col];
  }

  Vec4 operator*(Vec4 vector)
  {
    Vec4 result = {};
    int n1 = 4;
    int n2 = 4;

    int i, j; // i = row; j = column;

    // Load up A[n][n]
    for (i = 0; i<n2; i++)
    {
        for (j = 0; j<n1; j++)
        {
          result.values[j] += values[i][j] * vector[i];
        }
    }

    return result;
  }
 
  Mat4 operator*(Mat4 other)
  {
    Mat4 result = {};
    
    // TODO: think about how to do this in a for loop
    
    result.ax = ax * other.ax + bx * other.ay + cx * other.az + dx * other.aw;
    result.ay = ay * other.ax + by * other.ay + cy * other.az + dy * other.aw;
    result.az = az * other.ax + bz * other.ay + cz * other.az + dz * other.aw;
    result.aw = aw * other.ax + bw * other.ay + cw * other.az + dw * other.aw;
    
    result.bx = ax * other.bx + bx * other.by + cx * other.bz + dx * other.bw;
    result.by = ay * other.bx + by * other.by + cy * other.bz + dy * other.bw;
    result.bz = az * other.bx + bz * other.by + cz * other.bz + dz * other.bw;
    result.bw = aw * other.bx + bw * other.by + cw * other.bz + dw * other.bw;
    
    result.cx = ax * other.cx + bx * other.cy + cx * other.cz + dx * other.cw;
    result.cy = ay * other.cx + by * other.cy + cy * other.cz + dy * other.cw;
    result.cz = az * other.cx + bz * other.cy + cz * other.cz + dz * other.cw;
    result.cw = aw * other.cx + bw * other.cy + cw * other.cz + dw * other.cw;
    
    result.dx = ax * other.dx + bx * other.dy + cx * other.dz + dx * other.dw;
    result.dy = ay * other.dx + by * other.dy + cy * other.dz + dy * other.dw;
    result.dz = az * other.dx + bz * other.dy + cz * other.dz + dz * other.dw;
    result.dw = aw * other.dx + bw * other.dy + cw * other.dz + dw * other.dw;
    
    return result;
  }
};

Mat4 mat_4(float value)
{
  Mat4 result = {};
  result[0][0] = value;
  result[1][1] = value;
  result[2][2] = value;
  result[3][3] = value;

  return result;
}

// Mat4 translate(Mat4 m, Vec3 v)
// {
//   Mat4 result = mat_4(1.0f);
//   result[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
//   return result;
// }

// Mat4 look_at(Vec3 eye, Vec3 target, Vec3 up)
// {
//   Vec3 zaxis = normalize(target - eye);
//   Vec3 xaxis = normalize(cross(zaxis, up));
//   Vec3 yaxis = cross(xaxis, zaxis);
  
//   Mat4 result = mat_4(1.0f);
//   result[0][0] = xaxis.x;
//   result[1][0] = xaxis.y;
//   result[2][0] = xaxis.z;
//   result[0][1] = yaxis.x;
//   result[1][1] = yaxis.y;
//   result[2][1] = yaxis.z;
//   result[0][2] = -zaxis.x;
//   result[1][2] = -zaxis.y;
//   result[2][2] = -zaxis.z;
//   result[3][0] = -dot(xaxis, eye);
//   result[3][1] = -dot(yaxis, eye);
//   result[3][2] = dot(zaxis, eye);
  
//   return result;
// }

// Mat4 calculate_view_matrix(Vec3 pos, Vec3 front, Vec3 up)
// {
//   Mat4 view = look_at(pos, pos + front, up);
//   return view;
// }

Mat4 calculate_inverse_projection_matrix(float aspectRatio, float fov = 70.0f)
{
  // Projection matrix data to create an infinite reverse projection matrix
  float const zNear = 0.1f;
  float const aspect = aspectRatio;
  float const range = tanf(fov / 2.0f);
  float const left = -range * aspect;
  float const right = range * aspect;
  float const bottom = -range;
  float const top = range;
  
  Mat4 proj = mat_4(0.0f);
  proj[0][0] = 2.0f / (right - left);
  proj[1][1] = 2.0f / (top - bottom);
  proj[2][3] = -1.0f;
  proj[3][2] = zNear;
  proj[1][1] *= -1; // might not need it, this is vulkan specific, inverse y

  return proj;
}

Mat4 orthographic_projection(float left, float right, float top, float bottom)
{
  const float far = 1;
  const float near = 0;
  Mat4 result = {};
  result.aw = -(right + left) / (right - left); 
  result.bw = (top + bottom) / (top - bottom);
  result.cw = 0.0f; // Near Plane
  result[0][0] = 2.0f / (right - left);
    // matrix[0][0] = 2.0f / (right - left);

  result[1][1] = 2.0f / (top - bottom);
    // matrix[1][1] = 2.0f / (top - bottom);

  result[2][2] = 1.0f / (far - near);
  // matrix[2][2] = -2.0f / (far - near);

  result[3][3] = 1.0f;
    // matrix[3][3] = 1.0f;

  // result[0][3] = -(right + left) / (right - left);
  // result[1][3] = -(top + bottom) / (top - bottom);
  // result[2][3] = -(far + near) / (far - near);

  return result;
}

// #############################################################################
//                           Memeory Management
// #############################################################################
struct BumpAllocator
{
  size_t capacity;
  size_t used;
  char* memory;
};

BumpAllocator make_bump_allocator(size_t size)
{
  BumpAllocator result = {};

  size_t alignedSize = (size + 7) & ~7;
  result.capacity = alignedSize;
  result.memory = (char*)malloc(alignedSize);

  if(result.memory)
  {
    memset(result.memory, 0, alignedSize);
  }
  else
  {
    ASSERT_MSG(false, "Failed to malloc memory: %d", (int)size);
  }

  return result;
}

char* bump_alloc(BumpAllocator* allocator, size_t size)
{
  char* result = nullptr;

  size_t alignedSize = (size + 7) & ~7;
  if(allocator->used + alignedSize <= allocator->capacity)
  {
    result = allocator->memory + allocator->used;
    allocator->used += alignedSize;
  }
  else
  {
    ASSERT_MSG(0, "Bump allocator is full");
  }

  return result;
}

// #############################################################################
//                           File I/O
// #############################################################################
long long get_timestamp(const char* fileName)
{
  SDL_PathInfo info; 
  if (SDL_GetPathInfo(fileName, &info)) 
  {
    SDL_Time mtime = info.modify_time; // nanoseconds since Unix epoch 
    return mtime;
  }

  return 0;
}

long get_file_size(const char* filePath)
{
  ASSERT_MSG(filePath, "No filePath supplied!");

  SDL_PathInfo info; 
  if (SDL_GetPathInfo(filePath, &info)) 
  {
    Uint64 size = info.size;
    return size;
  }

  return 0;
}

void delete_file(const char* fileName)
{
  ASSERT_MSG(fileName, "No file name supplied!");
  SDL_RemovePath(fileName);
}

bool file_exists(const char* filePath)
{
  const bool exists = SDL_GetPathInfo(filePath, nullptr);
  return exists;
}

/*
* Reads a file into a supplied buffer. We manage our own
* memory and therefore want more control over where it 
* is allocated
*/
char* read_file(const char* filePath, int* fileSize, char* buffer)
{
  ASSERT_MSG(filePath, "No filePath supplied!");
  ASSERT_MSG(fileSize, "No fileSize supplied!");

  *fileSize = 0;

  SDL_IOStream* file = SDL_IOFromFile(filePath, "rb");
  if(!file)
  {
    SDL_Log("Failed opening File: %s", filePath);
    return nullptr;
  }

  SDL_SeekIO(file, 0, SDL_IO_SEEK_END);
  *fileSize = SDL_TellIO(file);
  SDL_SeekIO(file, 0, SDL_IO_SEEK_SET);

  memset(buffer, 0, *fileSize + 1);
  SDL_ReadIO(file, buffer, *fileSize);

  SDL_CloseIO(file);

  return buffer;
}

char* read_file(const char* filePath, int* fileSize, BumpAllocator* bumpAllocator)
{
  char* file = 0;
  long fileSize2 = get_file_size(filePath);

  if(fileSize2)
  {
    char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);

    file = read_file(filePath, fileSize, buffer);
  }

  return file; 
}

// #############################################################################
//                           Array
// #############################################################################
int max(int a, int b);

template<typename T, int N>
struct Array
{
  static constexpr int maxElements = N;
  int count = 0;
  T elements[N];

  T* begin()
  {
    if(count == 0)
    {
      return nullptr;
    }
    return elements;
  }

  const T* begin() const 
  {
    if(count == 0)
    {
      return nullptr;
    }
    return elements;
  }

  T* end()
  {
    if(count == 0)
    {
      return nullptr;
    }
    return (&elements[0]) + count;
  }

  const T* end() const 
  {
    if(count == 0)
    {
      return nullptr;
    }
    return (&elements[0]) + count;
  }

  T& operator[](int idx)
  {
    ASSERT_MSG(idx >= 0, "idx negative!");
    ASSERT_MSG(idx < count, "Idx out of bounds!");
    return elements[idx];
  }

  const T& operator[](int idx) const 
  {
    ASSERT_MSG(idx >= 0, "idx negative!");
    ASSERT_MSG(idx < count, "Idx out of bounds!");
    return elements[idx];
  }

  int add(const T& element)
  {
    ASSERT_MSG(count < maxElements, "Array Full!");
    ASSERT_MSG(count >= 0, "Count negative!");
    elements[count] = element;
    return count++;
  }

  void remove_idx_and_swap(int idx)
  {
    ASSERT_MSG(idx >= 0, "idx negative!");
    ASSERT_MSG(idx < count, "idx out of bounds!");
    elements[idx] = elements[--count];
  }

  void remove_idx_and_pack(int idx)
  {
    ASSERT_MSG(idx >= 0, "idx negative!");
    ASSERT_MSG(idx < count, "idx out of bounds!");

    int nextIdx = (idx + 1);
    if(nextIdx >= count)
    {
      count--;
      return;
    }

    int copyCount = count - nextIdx;
    memcpy(&elements[idx], &elements[nextIdx], copyCount * sizeof(T));
    count--;
  }

  int max_size()
  {
    return maxElements;
  }

  T& first()
  {
    return elements[0];
  }

  T& last()
  {
    return elements[max(count - 1, 0)];
  }

  void clear()
  {
    count = 0;
  }

  void pop()
  {
    if(count > 0)
    {
      count--;
    }
  }

  int get(const T& element) const
  {
    for(int idx = 0; idx < count; idx++)
    {
      if(elements[idx] == element)
      {
        return idx;
      }
    }

    return INVALID_IDX;
  }

  T* get_element(const T& element)
  {
    for(int idx = 0; idx < count; idx++)
    {
      if(elements[idx] == element)
      {
        return &elements[idx];
      }
    }

    return nullptr;
  }

  bool contains(const T& element) const
  {
    for(int idx = 0; idx < count; idx++)
    {
      if(elements[idx] == element)
      {
        return true;
      }
    }

    return false;
  }

  bool is_full()
  {
    return count >= N;
  }
};

// #############################################################################
//                           Config Parser
// #############################################################################
struct INIField
{
  char name[32];
  char value[128];
};

struct ConfigINI
{
  const char* fileName;
  Array<INIField, 200> fields;
};

ConfigINI parse_ini(const char* fileName, BumpAllocator* transientStorage)
{
  int fileSize = 0;
  char* data = read_file(fileName, &fileSize, transientStorage);
  
  if(!data)
  {
    ASSERT_MSG(false, "Couldn't read ini file %s", fileName);
  }

  ConfigINI configIni = {};
  configIni.fileName = fileName;

  INIField iniField = {};

  int nameLen = 0;
  int valueLen = 0;
  char* fieldName = data;
  char* fieldValue = nullptr;

  while(char c = *(data++))
  {
    // ini file example
    // \r\n
    // Section\r\n
    // resolution_x =    1920\r\n
    // resolution_y = 1080\r\n

    switch(c)
    {
      // Comment
      case ';': 
      {
        if(nameLen == 0)
        {
          // Skip until Newline
          while(char c = *(data++))
          {
            if(c == '\n' || c == 0)
            {
              break;
            }
          }

          fieldName = data;
        }

        break;
      }

      // Ignore windows bullshit
      case '\r':
      {
        continue;
      }

      // Newline
      case '\n':
      {
        if(fieldName != nullptr && fieldValue != nullptr)
        {
          iniField = {};
          memcpy(iniField.name, fieldName, nameLen);
          memcpy(iniField.value, fieldValue, valueLen);
          configIni.fields.add(iniField);

          nameLen = 0;
          valueLen = 0;
          fieldName = nullptr;
          fieldValue = nullptr;
        }

        fieldName = data;
        break;
      }

      // Seperator
      case '=':
      {
        fieldValue = data;
        while(char c = *(data++))
        {
          if(c == '\r') // Skip windows bullshit
          {
            continue;
          }
          if(c == '\n' || c == 0) // Skip until end of line or end of file
          {
            data--;
            break;
          }

          valueLen++;
        }
        break;
      }

      default: // Letters
      {
        nameLen++;
      }
    }
  }

  if(fieldName != nullptr && fieldValue != nullptr)
  {
    iniField = {};
    memcpy(iniField.name, fieldName, nameLen);
    memcpy(iniField.value, fieldValue, valueLen);
    configIni.fields.add(iniField);
  }

  return configIni;
}