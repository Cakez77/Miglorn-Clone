#version 300 es

// Input
layout (location = 0) in vec2 atlasPosIn;
layout (location = 1) in vec2 spriteSizeIn;
layout (location = 2) in vec2 posIn;
layout (location = 3) in vec2 sizeIn;

// Output
flat out vec2 spriteSize;
out vec2 textureCoords;
out vec2 uv;

struct GlobalData
{
  float gameTime;
  float padding;
  ivec2 windowSize;
  mat4 orthProjGame;
};

// Buffers
// uniform GlobalDataUBO globalData;
layout (std140) uniform GlobalDataUniform
{
  GlobalData globalData;
};

void main()
{
  vec2 vertices[6] = vec2[6](
    posIn,                                          // Top Left
    vec2(posIn.x           , posIn.y + sizeIn.y),   // Bottom Left
    vec2(posIn.x + sizeIn.x, posIn.y           ),   // Top Right
    vec2(posIn.x + sizeIn.x, posIn.y           ),   // Top Right
    vec2(posIn.x           , posIn.y + sizeIn.y),   // Bottom Left
    vec2(posIn.x + sizeIn.x, posIn.y + sizeIn.y)    // Bottom Right
  );

  vec2 localUVs[6] = vec2[6](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1)
  );

  // gl_VertexID is the index into the vertices when calling glDraw
  vec4 vertexPos = globalData.orthProjGame * vec4(vertices[gl_VertexID], 1, 1);
                   // globalData.orthoProjection * vec4(vertexPos, transform.layer, 1.0);
  gl_Position = vertexPos;

  uv = localUVs[gl_VertexID];

  //###########################################################
  //                    Output 
  //###########################################################

  // Sprite Size
  spriteSize = spriteSizeIn;

  // Texture Coords
  float left = atlasPosIn.x;
  float top = atlasPosIn.y;
  float right = atlasPosIn.x + spriteSizeIn.x;
  float bottom = atlasPosIn.y + spriteSizeIn.y;
  vec2 GPU_COORDS[6] = vec2[6](
    vec2(left,  top),
    vec2(left,  bottom),
    vec2(right, top),
    vec2(right, top),
    vec2(left,  bottom),
    vec2(right, bottom)
  );
  textureCoords = GPU_COORDS[gl_VertexID];



  // if(bool(transform.renderOptions & RENDERING_OPTION_FLIP_X))
  // {
  //   float tmpLeft = left;
  //   left = right;
  //   right = tmpLeft;
  // }

  // if(bool(transform.renderOptions & RENDERING_OPTION_FLIP_Y))
  // {
  //   float tmpTop = top;
  //   top = bottom;
  //   bottom = tmpTop;
  // }

  // OpenGL Device Coordinates
  // -1 / 1                          1 / 1
  // -1 /-1                          1 /-1
  // vec2 vertices[6] = vec2[6](
  
  //   vec2(-1,  1),  // Top Left
  //   vec2(-1, -1),  // Bottom Left
  //   vec2( 1,  1),  // Top Right
  //   vec2( 1,  1),  // Top Right
  //   vec2(-1, -1),  // Bottom Left
  //   vec2( 1, -1)   // Bottom Right
  // );

  // Rotation
  // {
  //   float angle = transform.angle;
  //   vec2 offset = transform.pos + transform.size / 2.0;
    
  //   for (int i = 0; i < 6; i++)
  //   {
  //     float newX = (vertices[i].x - offset.x) * cos(angle) - 
  //     (vertices[i].y - offset.y) * sin(angle);
      
  //     float newY = (vertices[i].x - offset.x) * sin(angle) + 
  //     (vertices[i].y - offset.y) * cos(angle);
      
  //     vertices[i].xy = vec2(newX + offset.x, newY + offset.y);
  //   }
  // }

  // vec4 position = 
  //   globalData.orthoProjection * 
  //   vec4(vertexPos, transform.layer, 1.0);
  // gl_Position = position;
  // worldPos = vertexPos;
  // ndc = position.xy;
  // localUVsOut = localUVs[gl_VertexID];

  // textureCoordsOut = textureCoords[gl_VertexID];
  // renderOptionsOut = transform.renderOptions;
  // materialIdx = transform.materialIdx;
  // spriteSize = transform.spriteSize;
  // atlasOffset= transform.atlasOffset;
  // quadSize = transform.size;

  // Scale cliprect to the screen
  // clipRect = transform.clipRect;
}