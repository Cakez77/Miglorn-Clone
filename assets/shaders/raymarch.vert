#version 300 es

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

// Input
layout (location = 0) in vec2 posIn;

// Output
out vec2 textureCoords;
out vec2 uv;

void main()
{
  vec2 localUVs[6] = vec2[6](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1)
  );

  mat4 orthoProj = globalData.orthProjGame[0];
  vec4 vertexPos = orthoProj * vec4(posIn, 1, 1);
  gl_Position = vertexPos;
  uv = localUVs[gl_VertexID];
}