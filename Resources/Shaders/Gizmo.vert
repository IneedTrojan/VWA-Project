#version 450

layout(location = 0) in
vec3 aPos;

out VS_OUT {
    uint color;
} vs_out;

layout(std430, binding = 1)
buffer transformBuffer
{
    mat4 transforms[];
};
layout(std430, binding = 2)
buffer colorBuffer
{
    uint colors[];
};

struct Camera{
    mat4 modelSpaceMatrix;
	mat4 viewSpaceMatrix;
	mat4 camSpaceMatrix;
};

layout(std430, binding = 0) buffer CameraSSBO {
    Camera cameraMatrices[];
};



void main()
{
    Camera cam = cameraMatrices[0];

    mat4 model = transforms[gl_InstanceID];
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = cam.camSpaceMatrix * worldPos;
    vs_out.color = colors[gl_InstanceID];

}
