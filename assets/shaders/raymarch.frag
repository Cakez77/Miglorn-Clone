#version 300 es
precision highp float;

// Input
in vec2 textureCoords;
in vec2 uv;

// Output
layout (location = 0) out vec4 fragColor;

uniform sampler2D textureAtlas;

void main()
{
  // vec4 textureColor = texture(textureAtlas, uv);
  vec4 textureColor = texelFetch(textureAtlas, ivec2(textureCoords), 0);
  vec4 color = vec4(1.0);
  // if(textureColor.a == 0.0)
  // {
  //   discard;
  // }
  fragColor = color; 
}
