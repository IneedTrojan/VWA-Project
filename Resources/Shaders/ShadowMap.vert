#version 450 core

layout (location = 0) in
vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 clipPos = lightSpaceMatrix * worldPos;
    gl_Position = clipPos;
}
