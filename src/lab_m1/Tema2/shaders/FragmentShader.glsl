#version 330

// Input
in vec3 frag_color;

// Output
layout(location = 0) out vec4 out_color;

uniform float timeFactor;


void main()
{
    vec3 red_color = vec3(1.0, 0.0, 0.0);
    vec3 finalColor = mix(red_color, frag_color, timeFactor);
    out_color = vec4(finalColor, 1);
}
