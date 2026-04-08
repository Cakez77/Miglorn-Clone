#pragma once
#include <SDL3/SDL.h>

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

#define u8 uint8_t
#define s8 int8_t
#define u16 uint16_t
#define s16 int16_t
#define u32 uint32_t
#define i32 int32_t
#define u64 uint64_t
#define s64 int64_t

#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))

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
  operator bool() {return x != 0.0f || y != 0.0f;}
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
