#version 450

in GS_OUT {
    vec2 uv;
    vec3 worldPosition;
    vec4 debugColor;
    vec3 normal;
    vec3 lerp;
    float debug_area;

} vs_in;




layout(location = 0) out
vec4 Albedo;
layout(location = 1) out
vec3 WorldNormal;
layout(location = 3) out
vec4 WorldPosition;
layout(location = 4) out
vec3 RSM;

void main() {
    vec2 uv = vs_in.uv;
    

    vec3 lerp = vs_in.lerp;
    float debug_area = vs_in.debug_area;

    float edgeThreshold = 1.0 / sqrt(debug_area) / 5.0; 

    float minBaryCoord = min(min(lerp.x, lerp.y), lerp.z);

    float edgeFactor = smoothstep(edgeThreshold, edgeThreshold * 2.0, minBaryCoord);

    vec4 baseColor = vs_in.debugColor;

    vec4 color = baseColor;//mix(baseColor*0.7, vec4(1, 1, 1, 1), 1.0 - edgeFactor);

    vec3 normal = vs_in.normal;



    Albedo = color;
    WorldNormal = normal/2+0.5;
    WorldPosition = vec4((vs_in.worldPosition + 25000.0) / 50000.0, 1.0);
    RSM = vec3(0.5, 0.2, 1.0);
}
