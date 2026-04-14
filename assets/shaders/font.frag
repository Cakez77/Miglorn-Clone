#version 300 es
precision highp float;

// Input
flat in vec2 spriteSize;
flat in int fontIdx;
in vec2 textureCoords;
in vec2 uv;

// Output
layout (location = 0) out vec4 fragColor;

// Textures
uniform mediump sampler2DArray fontArray;

void main()
{
  vec4 textureColor = texelFetch(fontArray, ivec3(textureCoords, fontIdx), 0);

  bool outline = false;
  float channel = textureColor.g;
  if(outline)
  {
    channel = textureColor.r;
  }

  if(channel == 0.0)
  {
    discard;
  }

  vec4 fontColor = vec4(0.0,0.0,0.0,1.0);
  if(textureColor.g > 0.0)
  {
    fontColor = vec4(1.0,1.0,1.0,1.0);
  }

  fragColor = fontColor; 
}
