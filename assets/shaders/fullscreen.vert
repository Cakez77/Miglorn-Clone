#version 300 es

// Input - nothing

// Output
out vec2 uv;
// flat out vec2 spriteSize;
// out vec2 textureCoords;

// struct GlobalData
// {
//   float gameTime;
//   float padding;
//   ivec2 windowSize;
//   mat4 orthProjGame;
// };

// // Buffers
// // uniform GlobalDataUBO globalData;
// layout (std140) uniform GlobalDataUniform
// {
//   GlobalData globalData;
// };

void main()
{
  // OpenGL Device Coordinates
  // -1 / 1                          1 / 1
  // -1 /-1                          1 /-1
  vec2 VERTICES[6] = vec2[6](
  
    vec2(-1,  1),  // Top Left
    vec2(-1, -1),  // Bottom Left
    vec2( 1,  1),  // Top Right
    vec2( 1,  1),  // Top Right
    vec2(-1, -1),  // Bottom Left
    vec2( 1, -1)   // Bottom Right
  );

  vec2 UVs[6] = vec2[6](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1)
  );

  gl_Position = vec4(VERTICES[gl_VertexID], 1, 1);
  uv = UVs[gl_VertexID];
}