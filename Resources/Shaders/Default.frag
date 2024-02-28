#version 450 core

in GS_OUT {
    mat3 TBN;
    vec3 fragPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    vec3 worldPosition;
    vec2 uv;
    flat float debug_area;
    vec3 triangleCoordinates;
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


uniform samplerCube cubemap;


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



    //check if it is on edge of triangle using the vec3 triangleCoordinates

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
    vec3 worldViewDir = normalize(fs_in.worldPosition - cam.modelSpaceMatrix[3].xyz);
    vec3 tangentViewDir = normalize(fs_in.TBN * worldViewDir);
    vec2 uv = fs_in.worldPosition.xy / 200.0;

    vec3 unpackedTextureNormal = texture(normalTexture, uv).xyz * 2.0 - 1.0;
    vec3 normal = normalize(fs_in.TBN * vec3(0, 0, 1)); // Assuming fs_in.TBN * vec3(0, 0, 1) gives the normal in world space.
    vec3 packedNormal = normal * 0.5 + 0.5;
    vec4 packedPosition = vec4((fs_in.worldPosition + 25000.0) / 50000.0, 1.0);
    vec3 unpackedPosition = packedPosition.xyz * 20.0 - 10.0;

    // Determine steepness
    float steepness = dot(normal, vec3(0, 1, 0)); // Dot product with up vector
    steepness = clamp(steepness, 0.0, 1.0); // Ensure it's within [0, 1]


    float edgeThreshold = 1.0 / sqrt(fs_in.debug_area) / 5.0; 

    float minBaryCoord = min(min(fs_in.triangleCoordinates.x, fs_in.triangleCoordinates.y), fs_in.triangleCoordinates.z);

    float edgeFactor = smoothstep(edgeThreshold, edgeThreshold * 2.0, minBaryCoord);

    vec4 baseColor;
    if (steepness > 0.5) {
        baseColor = mix(vec4(0.5, 0.5, 0.5, 1.0), vec4(1.0, 1.0, 1.0, 1.0), (steepness - 0.75) * 4.0);
    } else if (steepness > 0.4) {
        baseColor = mix(vec4(0.6, 0.4, 0.2, 1.0), vec4(0.5, 0.5, 0.5, 1.0), (steepness - 0.5) * 4.0);
    } else if (steepness > 0.3) {
        baseColor = mix(vec4(0.0, 0.5, 0.0, 1.0), vec4(0.6, 0.4, 0.2, 1.0), (steepness - 0.25) * 4.0);
    } else {
        baseColor = mix(vec4(0.0, 0.8, 0.0, 1.0), vec4(0.0, 0.5, 0.0, 1.0), steepness * 4.0);
    }

    vec4 color = mix(baseColor*0.7, vec4(1, 1, 1, 1), 1.0 - edgeFactor);
    packedNormal = vec3(0.5,0.5,0.5);


    Albedo = color;
    WorldNormal = packedNormal;
    WorldPosition = packedPosition;
}





//
//
//void main()
//{
//    
//    Camera cam = cameraMatrices[0];
//
//    vec3 worldViewDir = normalize(fs_in.worldPosition -cam.modelSpaceMatrix[3].xyz);
//
//    vec3 tangentViewDir = normalize(fs_in.TBN * worldViewDir);
//    vec2 uv = fs_in.worldPosition.xy/200.0;//calculateUniplanarUV(fs_in.worldPosition, fs_in.TBN[2], gl_FragCoord.xy, 16, iTime);
//
//    //uv = calculateParallaxUV(displacementTexture, uv, worldViewDir, 0.05, 8.0, 64.0);
//
//    vec3 unpackedTextureNormal = texture(normalTexture, uv).xyz * 2.0 - 1;
//    vec3 normal = fs_in.TBN*vec3(0,0,1);//fs_in.TBN * unpackedTextureNormal;
//    vec3 packedNormal = normal / 2.0 + 0.5;
//    vec4 packedPosition = vec4((fs_in.worldPosition + 25000.0) / 50000.0, 1.0);
//    vec3 unpackedPosition = packedPosition.xyz * 20.0 - 10.0;
//
//    float base = 4.0;
//    float squared = base*base;
//    float score = fs_in.debug_area/255.0;
//
//    Albedo = vec4(score.xxx,1);
//    WorldNormal = packedNormal;
//    WorldPosition = packedPosition;
//
//
//}