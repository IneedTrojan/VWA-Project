#version 450 core

in VS_OUT {
    mat3 TBN;
    vec3 fragPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    vec3 worldPosition;
    vec2 uv;
} fs_in; 

layout(location = 0) out
vec4 Albedo;
layout(location = 1) out
vec3 WorldNormal;
layout(location = 3) out
vec4 WorldPosition;
layout(location = 4) out
vec3 RSM;


uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D metallicTexture;
uniform sampler2D displacementTexture;

uniform float iTime;



struct Camera{
    mat4 modelSpaceMatrix;
	mat4 viewSpaceMatrix;
	mat4 camSpaceMatrix;
};

layout(std430, binding = 0) buffer CameraSSBO {
    Camera cameraMatrices[];
};




vec2 calculateParallaxUV(sampler2D displacementTexture, vec2 uv, vec3 viewDirection, float heightScale, float minLayers, float maxLayers) {


    viewDirection = normalize(viewDirection);
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0,0,1), viewDirection)));//what kind of vector is vec3(1,0,0) here is it tangent?
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDirection.xz / viewDirection.y * heightScale;
    vec2 deltaUV = P / numLayers;

    vec2 currentUV = uv;
    float currentDepthValue = texture(displacementTexture, currentUV).r;

    while (currentLayerDepth < currentDepthValue) {
        currentUV -= deltaUV;
        currentDepthValue = texture(displacementTexture, currentUV).r;
        currentLayerDepth += layerDepth;
    }
//
//    vec2 prevUV = currentUV + deltaUV;
//    float prevLayerDepth = currentLayerDepth - layerDepth;
//    float afterDepth = currentDepthValue - currentLayerDepth;
//    float beforeDepth = texture(displacementTexture, prevUV).r - prevLayerDepth;
//    float weight = afterDepth / (afterDepth - beforeDepth);
//    vec2 finalUV = prevUV * weight + currentUV * (1.0 - weight);

    return currentUV;
}

float ScreenSpaceDither(vec2 screenPos,float iTime) {
    const vec2 ditherPattern = vec2(171.0, 231.0);
    float dither = dot(ditherPattern+iTime, screenPos-iTime); 
    return fract(dither / 103.0);
}


vec2 blendUV(vec2 uv1, vec2 uv2, float blend) {
    return mix(uv1, uv2, blend);
}


vec2 calculateUniplanarUV(vec3 worldPos, vec3 normal, vec2 screenPos, float contrast, float iTime) {
    vec3 powAlpha = pow(abs(normal), vec3(contrast));
    vec3 alpha = powAlpha / (powAlpha.x + powAlpha.y + powAlpha.z);

    float ditherValue = ScreenSpaceDither(screenPos, iTime);

    ditherValue = clamp(ditherValue, 0.01, 0.99);

   vec2 primaryUV = alpha.x > alpha.y ?
    (alpha.x > alpha.z ? worldPos.yz : worldPos.xy) 
        : (alpha.y > alpha.z ? worldPos.xz :
        worldPos.xy);

    return primaryUV;
}



void main()
{
    
    Camera cam = cameraMatrices[0];

    vec3 worldViewDir = normalize(fs_in.worldPosition -cam.modelSpaceMatrix[3].xyz);

    vec3 tangentViewDir = normalize(fs_in.TBN * worldViewDir);
    vec2 uv = calculateUniplanarUV(fs_in.worldPosition/20.0, fs_in.TBN[2], gl_FragCoord.xy, 16, iTime);

    uv = calculateParallaxUV(displacementTexture, uv, worldViewDir, 0.05, 8.0, 64.0);

    vec3 unpackedTextureNormal = texture(normalTexture, uv).xyz * 2.0 - 1;
    vec3 normal = fs_in.TBN * unpackedTextureNormal;
    vec3 packedNormal = normal / 2.0 + 0.5;
    vec4 packedPosition = vec4((fs_in.worldPosition + 100.0) / 200.0, 1.0);
    vec3 unpackedPosition = packedPosition.xyz * 20.0 - 10.0;

    Albedo =vec4(0.8,0.2,0.7,1.0);//vec4(fs_in.TBN[0], 1)*2+1;//vec4(texture(displacementTexture, uv).xxx,1);
    WorldNormal = packedNormal;
    WorldPosition = packedPosition;


}