
// Input
flat in vec2 spriteSize;
in vec2 textureCoords;
in vec2 uv;
flat in int renderOptions;
flat in uvec2 mat;

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

  if(bool(renderOptions & RENDER_OPTION_LINEAR_FILTERING))
  {
    vec2 uv = vec2(textureCoords) / vec2(textureSize(textureAtlas, 0));
    textureColor = texture(textureAtlas, uv);
    textureColor += gradientNoise(gl_FragCoord.xy) * 0.003;
  }

  if(bool(renderOptions & RENDER_OPTION_LIGHT_RAYMARCH))
  {
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
      float startDist = globalData.obstacles[i].dist;
      float fade = mix(10.0, 1.0, startDist*2.0);
      // float fade = 1.0;
      // float startDeg =  globalData.obstacles[i].startDeg - fade/2.0;
      // float endDeg =  globalData.obstacles[i].endDeg + fade/2.0;
      float startDeg =  globalData.obstacles[i].startDeg;
      float endDeg =  globalData.obstacles[i].endDeg;

      if(startDeg >= endDeg && (deg >= startDeg || deg <= endDeg) && lengthSquared > startDist)
      {
        endDeg += 360.0;
        if(deg < startDeg)
        {
          deg += 360.0;
        }
        // example:  startDeg = 345 & endDeg = 11
        float degDist = min(deg - startDeg, endDeg - deg);
        textureColor.a *= mix(1.0, 0.0, clamp(degDist/fade, 0.0, 1.0));
        break;
      }
      else if(deg >= startDeg && deg <= endDeg && lengthSquared > startDist)
      {
        float degDist = min(deg - startDeg, endDeg - deg);
        textureColor.a *= mix(1.0, 0.0, clamp(degDist/fade, 0.0, 1.0));
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


  Color emissive;
  emissive.hex = mat[0];
  Color addColor;
  addColor.hex = mat[1];


  Color myColor; 
  // myColor.hex = 0x1e6f50FF;
  myColor.hex = uint(0xFFFFFFFF);
  // myColor.hex = 0x00FF00FF;
  vec4 color = get_color(myColor.hex);
  vec4 colorEmissive = get_color(emissive.hex);
  vec4 colorAdd = get_color(addColor.hex);
  // uint test = (128u << 24) + (128u << 16) + (128u << 8) + 128u;
  // vec4 colorAdd = get_color(4286348412u);
  // vec4 colorAdd = get_color(test);
  // colorAdd = vec4(180.0/255.0, 180.0/255.0, 0.0, 1.0);

  // Output
  fragColor = textureColor * colorEmissive - colorAdd;
}
