#version 300 es
precision highp float;

// Input
flat in vec2 spriteSize;
in vec2 textureCoords;
in vec2 uv;

// layout (location = 0) in vec2 textureCoordsIn;
// layout (location = 1) in flat int renderOptions;
// layout (location = 2) in flat int materialIdx;
// layout (location = 3) in flat vec4 clipRect;
// layout (location = 4) in vec2 worldPos;
// layout (location = 5) in vec2 localUVs;
// layout (location = 6) in flat ivec2 spriteSize;
// layout (location = 7) in flat ivec2 atlasOffset;
// layout (location = 8) in vec2 quadSize;
// layout (location = 9) in vec2 ndc;

// Output
layout (location = 0) out vec4 fragColor;
// layout (location = 1) out vec4 bloomColor;

uniform sampler2D textureAtlas;

void main()
{
  // vec4 textureColor = texture(textureAtlas, uv);
  vec4 textureColor = texelFetch(textureAtlas, ivec2(textureCoords), 0);

  if(textureColor.a == 0.0)
  {
    discard;
  }

  // textureColor.r += spriteSize.x * 00000.1;

  fragColor = textureColor; 
  // vec4(1,1,1,1);
}
