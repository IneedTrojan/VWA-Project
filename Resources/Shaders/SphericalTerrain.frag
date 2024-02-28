#version 450

in VS_OUT {
    vec4 debugColor;
    vec3 worldPosition;
} fs_in;

layout(location = 0) out
vec4 Albedo;
layout(location = 1) out
vec3 WorldNormal;
layout(location = 3) out
vec4 WorldPosition;
layout(location = 4) out
vec3 RSM;

void main() {

    Albedo = fs_in.debugColor;
    WorldNormal = vec3(0.5,0.5,0.5);
    WorldPosition = vec4(0,0,0,0);
    RSM = vec3(0,0,0);
}
