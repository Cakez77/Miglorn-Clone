#version 300 es
precision highp float;

// Input
in vec2 uv;

// Output
layout (location = 0) out vec4 fragColor;

// Buffers
struct LightObstacle
{
  float startDeg;
  float endDeg;
  float dist;
  float padding;
};

struct GlobalData
{
  float gameTime;
  int lightObstacleCount;
  ivec2 windowSize;
  vec2 camPos;
  vec2 padding;
  mat4 orthProjGame[4];
  LightObstacle obstacles[10];
};

// Buffers
// uniform GlobalDataUBO globalData;
layout (std140) uniform GlobalDataUniform
{
  GlobalData globalData;
};

// Textures
uniform sampler2D lightMap; // Is this location 1?

void main()
{
  vec4 textureColor = texture(lightMap, uv);

  if(textureColor.a == 0.0)
  {
    discard;
  }

  fragColor = textureColor; 
}
