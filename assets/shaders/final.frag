#version 300 es
precision highp float;

// Input
in vec2 uv;

// Output
layout (location = 0) out vec4 fragColor;

uniform sampler2D uiTexture; // Is this location 1?

void main()
{
  vec2 uvTest = uv;
  // uvTest.y = 1.0 - uvTest.y;

  vec4 textureColor = texture(uiTexture, uvTest);

  // if(textureColor.a == 0.0)
  // {
  //   discard;
  // }

  fragColor = textureColor; 
}
