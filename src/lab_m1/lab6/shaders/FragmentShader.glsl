#version 330

// Input
// TODO(student): Get values from vertex shader
in vec3 frag_color;
in vec3 frag_texture;
in vec3 frag_normal;
in vec3 frag_position;

// Output
layout(location = 0) out vec4 out_color;

uniform float time;

void main()
{
    // TODO(student): Write pixel out color
    vec3 color;
    color.r = abs(sin(time + frag_color.r));
    color.g = abs(sin(time + frag_color.g));
    color.b = abs(sin(time + frag_color.b));

    vec3 colorStripe;
    colorStripe.r = abs(cos(time + frag_color.r));
    colorStripe.g = abs(cos(time + frag_color.g));
    colorStripe.b = abs(cos(time + frag_color.b));
    out_color = mix(vec4(color, 1.0),vec4(colorStripe,1.0),sin(frag_position.x * 10.0 + time * 5.0)/2);

}
