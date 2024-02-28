#version 450 core

layout(location = 0) in vec3 inPosition;

out VS_OUT {
    smooth vec3 normalizedPosition;
} vs_out;

struct Camera {
    mat4 modelSpaceMatrix;
    mat4 viewSpaceMatrix;
    mat4 camSpaceMatrix;
};

layout(std430, binding = 0) buffer CameraSSBO {
    Camera cameraMatrices[];
};

uniform mat4 camMatrix;
void main() {

    mat4 _camMatrix = cameraMatrices[0].camSpaceMatrix;
   // _camMatrix[3] = vec4(0,0,0,1);
    vec4 position = _camMatrix*vec4(inPosition*10.0, 1);
    vs_out.normalizedPosition = vec3(inPosition.xy, inPosition.z);

    gl_Position = position;
}

