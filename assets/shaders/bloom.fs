#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
// out vec4 finalColor;
// out vec4 bloomColor;
layout(location = 0) out vec4 finalColor;
layout(location = 1) out vec4 bloomColor;

// NOTE: Add your custom variables here

const vec2 size = vec2(800, 450);   // Framebuffer size
const float samples = 5.0;          // Pixels per axis; higher = bigger glow, worse performance
const float quality = 2.5;          // Defines size factor: Lower = smaller glow, better quality

float gradient(vec2 pos, vec2 uv, float size, float falloff)
{
	float d = distance(pos / size, uv / size);
	float result = max(0.0, 1.0 - pow(d, falloff));
	return result;
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 source = texture(texture0, fragTexCoord);
    finalColor = source;
    bloomColor = source;
}