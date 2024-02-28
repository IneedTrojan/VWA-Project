#version 450
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform uvec3 dispatchSize;

uniform sampler2D albedoTexture;
uniform sampler2D worldNormalTexture;
uniform sampler2D worldPositionTexture;
uniform sampler2D rsmTexture;
uniform sampler2D depthTexture;

uniform sampler2DShadow shadowMap;
uniform mat4 lightMatrix;

layout(binding = 0, rgba8) uniform writeonly image2D outColorTexture;


struct Camera{
    mat4 modelSpaceMatrix;
	mat4 viewSpaceMatrix;
	mat4 camSpaceMatrix;
};

layout(std430, binding = 2) buffer CameraSSBO {
    Camera cameraMatrices[];
};


bool IsInRange(float value, float min, float max) {

    return (value <=max) && (value>=min);
}
mat4 lookAt(vec3 eye, vec3 center, vec3 up) {
    vec3 z = normalize(eye - center);
    vec3 x = normalize(cross(up, z));
    vec3 y = cross(z, x);

    mat4 view = mat4(1.0);
    view[0][0] = x.x; view[1][0] = x.y; view[2][0] = x.z;
    view[0][1] = y.x; view[1][1] = y.y; view[2][1] = y.z;
    view[0][2] = z.x; view[1][2] = z.y; view[2][2] = z.z;
    view[3][0] = -dot(x, eye); view[3][1] = -dot(y, eye); view[3][2] = -dot(z, eye);

    return view;
}
void main() {

    Camera cam = cameraMatrices[0];

    ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texCoord) / vec2(dispatchSize.xy) / vec2(16, 16);
    vec4 albedo = texture(albedoTexture, uv);
    vec3 unpackedWorldPosition = texture(worldPositionTexture, uv).xyz * 50000.0 - 25000.0;
    vec3 unpackedWorldNormal = texture(worldNormalTexture, uv).xyz * 2.0 - 1.0;
    vec3 RSM = texture(rsmTexture, uv).xyz;

    vec4 litColor = albedo;
    
    vec3 lightPos = vec3(0.0, 10.0, 0.0); // Positioned above the scene
    vec3 center = vec3(0.0, 0.0, 0.0); // Looking at the origin
    vec3 up = vec3(0.0, 0.0, -1.0); // Up direction

    mat4 lightMatrixT = lookAt(lightPos, center, up);
 


    float depth = texture(depthTexture, uv).x;

    if (depth > 0.0 && depth < 1.0 && length(unpackedWorldNormal)>0.1) {

        vec4 lightSpacePos = lightMatrixT * vec4(unpackedWorldPosition, 1.0);
        lightSpacePos /= lightSpacePos.w;
        vec3 lightSpaceUV = lightSpacePos.xyz * 0.5 + 0.5;

        

        vec4 normalData = vec4(unpackedWorldNormal, 1);
        vec3 normal = normalize(normalData.rgb);

        vec3 lightDir = normalize(vec3(lightMatrixT[0][2], lightMatrixT[1][2], lightMatrixT[2][2]));
        float diff = max(dot(normal, lightDir), 0.0);
        litColor = albedo * clamp(diff, 0.2, 1.0);


        /*float arr[3];
        arr[0] = albedo.x;
        arr[1] = albedo.y;
        arr[2] = albedo.z;
        int max = int(arr[0] > 0.016) + int(arr[1] > 0.062) + int(arr[2] > 0.24);
        arr[0] = arr[0] * float(max == 0);
        arr[1] = arr[0] * float(max == 1);
        arr[2] = arr[0] * float(max == 2);*/

        
        
        /*bool outsideShadowMap = lightSpaceUV.x < 0.0 || lightSpaceUV.x > 1.0 ||
            lightSpaceUV.y < 0.0 || lightSpaceUV.y > 1.0||
            lightSpaceUV.z < -1.0 || lightSpaceUV.z > 1.0;

        if (!outsideShadowMap) {
            float shadow = texture(shadowMap, lightSpaceUV).r;
            litColor *= mix(vec4(0.2, 0.2, 0.2, 1), litColor, shadow); // Mix between shadowed and lit color
        }
        
        if (dst > 1.0) {
            dst = 1.0;
        }
        litColor *= (dst*0.2+0.8);*/


       /* vec3 u = lightSpaceUV;
        litColor = vec4(1.0-distance.xxx, 5.0)/5.0;
        if ((u.x < 0.05 || u.y < 0.05 || u.z < 0.05 || u.x >0.95 || u.y>0.95||u.z>0.95)
            && (IsInRange(u.x, 0, 1) && IsInRange(u.y, 0, 1) && IsInRange(lightSpaceUV.z, 0,1))
            ) {
            litColor = vec4(1, 1, 1, 1);
        }*/
    }


        imageStore(outColorTexture, texCoord, litColor);



#if 0
    if (depth > 0.0 && depth < 1.0) {

        #if 1
        //litColor = vec4(unpackedWorldNormal, 1.0);
       //litColor = vec4(lightSpaceUV, 1.0);
       //litColor = vec4(unpackedAlbedoPosition/200.0, 1.0);
        vec3 worldPosition = texture(worldPositionTexture, uv).xyz * 20000.0 - 10000.0;
        //vec3 unpackedPosition = packedPosition.xyz * 20000.0 - 10000.0;

        // Transform the world position to light space
       
        albedo = vec4(mod(worldPosition, 1.0), 1.0);
        litColor = vec4(worldPosition, 1.0);
        
#endif

        //litColor = vec4(worldPosition,1.0);

        /*
       
       

        // Check if the coordinates are outside the [0, 1] range
        bool outsideShadowMap = lightSpaceUV.x < 0.0 || lightSpaceUV.x > 1.0 ||
            lightSpaceUV.y < 0.0 || lightSpaceUV.y > 1.0;

       

        */
    }

    // Debug window to visualize the shadow map
    
#endif
}
