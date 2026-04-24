#version 300 es
precision highp float;

// Input
flat in vec2 spriteSize;
in vec2 textureCoords;
in vec2 uv;
flat in int renderOptions;

// Output
layout (location = 0) out vec4 fragColor;
// layout (location = 1) out vec4 bloomColor;

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

uniform sampler2D textureAtlas;

void main()
{
  // vec4 textureColor = texture(textureAtlas, uv);
  vec4 textureColor = texelFetch(textureAtlas, ivec2(textureCoords), 0);

  int RENDER_OPTION_LIGHT_RAYMARCH = 1;

  if(bool(renderOptions & RENDER_OPTION_LIGHT_RAYMARCH))
  {
    mat4 orthoProj = globalData.orthProjGame[0];
    vec2 camPos = (orthoProj * vec4(globalData.camPos, 0.0, 0.0)).xy;
    // vec2 st =  camPos;
    // vec2 st =  camPos + (uv * 2.0 - 1.0); // convert to ndc
    vec2 st = (uv * 2.0 - 1.0); // convert to ndc
    st.y = -st.y;
    float lengthSquared = dot(st, st);
    // lengthSquared *= lengthSquared;
    float PI = 3.1415926535;

    // Calculate the angle of the fragment
    float angle = atan(st.y, st.x);
    float deg = (angle + PI) * 180.0/PI; 

    // // Make the circle dimmer
    // material.color.a *= 0.3;

    for(int i = 0; i < globalData.lightObstacleCount; i++)
    {
      float dist = globalData.obstacles[i].dist;
      float startDeg =  globalData.obstacles[i].startDeg;
      float endDeg =  globalData.obstacles[i].endDeg;

      if(startDeg >= endDeg)
      {
        if(deg >= startDeg && lengthSquared > dist)
        {
          // Blocked vision
          textureColor.a = 0.0;
        }

        if(deg <= endDeg && lengthSquared > dist)
        {
          // Blocked vision
          textureColor.a = 0.0;
        }
      }
      else if(deg >= startDeg && deg <= endDeg && lengthSquared > dist)
      {
        textureColor.a = 0.0;
      }
    }
  }

  if(textureColor.a == 0.0)
  {
    discard;
  }

  // textureColor.r += spriteSize.x * 00000.1;
  // material.color.r = pow(abs(material.color.r), 2.2);
  // material.color.g = pow(abs(material.color.g), 2.2);
  // material.color.b = pow(abs(material.color.b), 2.2);
  // material.color.a = pow(abs(material.color.a), 2.2);

  // textureColor.r -= pow(180.0/255.0, 2.0); 
  // textureColor.g -= pow(180.0/255.0, 2.0); 
  // textureColor.b -= 0.0; 

  fragColor = textureColor; 
  // vec4(1,1,1,1);
}
