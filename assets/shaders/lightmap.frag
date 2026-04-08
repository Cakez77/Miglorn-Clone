#version 300 es
precision highp float;

// Input
in vec2 uv;

// Output
layout (location = 0) out vec4 fragColor;

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
