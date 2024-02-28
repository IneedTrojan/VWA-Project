#version 450
layout (location = 0) in
vec3 aPos;

mat4 model;
mat4 camSpaceMatrix;

out
vec4 color;

void main()
{
    gl_Position = camSpaceMatrix * (model*vec4(aPos,1.0));
}