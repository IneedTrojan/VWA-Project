#version 450 core





layout(location = 0) in
vec3 inPosition;
layout(location = 1) in
vec3 inNormal;
layout(location = 3) in
vec2 inUv;
layout(location = 2) in
vec4 inTangent;

out VS_OUT {
    mat3 TBN;
    vec3 fragPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    vec3 worldPosition;
    vec2 uv;
} vs_out;

struct Camera {
    mat4 modelSpaceMatrix;
    mat4 viewSpaceMatrix;
    mat4 camSpaceMatrix;
};

layout(std430, binding = 0) buffer CameraSSBO {
    Camera cameraMatrices[];
};

uniform mat4 model;
uniform vec3 lightPos;
uniform float iTime;




dvec3 cubeSphereProjection(dvec3 worldPosition, double radius, double blend)
{
    worldPosition = worldPosition / radius;

    dvec3 squared = { worldPosition.x * worldPosition.x, worldPosition.y * worldPosition.y, worldPosition.z * worldPosition.z };

    double x = worldPosition.x * sqrt(1.0 - (squared.y + squared.z) / 2.0 + (squared.y * squared.z) / 3.0) * radius;
    double y = worldPosition.y * sqrt(1.0 - (squared.x + squared.z) / 2.0 + (squared.x * squared.z) / 3.0) * radius;
    double z = worldPosition.z * sqrt(1.0 - (squared.x + squared.y) / 2.0 + (squared.x * squared.y) / 3.0) * radius;


    return dvec3(x, y, z) * blend + worldPosition * (1.0-blend)*radius;
}










void main()
{
    // Extract the camera matrices
    Camera cam = cameraMatrices[0];

    // Calculate world position of the vertex
    vec4 worldPos = model * vec4(inPosition, 1.0);

    float t = iTime/10.0;
    float radius = 100;
    float blend = smoothstep(0.2, 0.8, clamp(mod(t, 2.0), 0, 1)-clamp(mod(t, 2.0) - 1, 0, 1));
    worldPos = vec4(cubeSphereProjection(worldPos.xyz, radius, 0),1);

    // Calculate clip space position for the vertex
    vec4 clipPos = cam.camSpaceMatrix * worldPos;

    // Normalize the T, B, and N vectors and calculate TBN matrix
    vec3 T = normalize(vec3(model * vec4(inTangent.xyz, 0.0)));
    vec3 N = normalize(vec3(model * vec4(inNormal, 0.0)));
    vec3 B = cross(N, T); // Assumed tangent.w (handedness) is +1 or -1 to flip B if necessary

    mat3 TBN = mat3(T, B, N); // Construct the TBN matrix

    

    // Set outputs
    vs_out.TBN = TBN;
    vs_out.fragPos = worldPos.xyz; // or just inPosition if you want the local position
    vs_out.worldPosition = worldPos.xyz;
    vs_out.tangentFragPos = TBN * worldPos.xyz; // Transform world position to tangent space
    vs_out.tangentViewPos = TBN * (cam.viewSpaceMatrix * worldPos).xyz; // Transform view position to tangent space
    vs_out.uv = vec2(0,0);

    gl_Position = clipPos;
}