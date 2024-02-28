//#version 450 core
//
//layout(binding = 0, rgba8) writeonly uniform image2D forward;
//
//uniform sampler2DArray equirectTexture;
//
//#define PI 3.1415926535
//
//layout(local_size_x = 16, local_size_y = 16) in;
//
//
//
//uniform ivec2 cubemap_res;
//uniform mat3 tbn;
//uniform vec4 debugColor;
//uniform vec4 tint;
//uniform vec2 equirectTextureGridSize;
//
//
//
//
//vec3 calcPosition(vec2 uv, mat3 tbn){
//    
//    vec3 cubePos = tbn*vec3(uv*2-1, 1);
//    return normalize(cubePos);
//}
//vec2 longlat(vec3 point) {
//
//    float longitude = atan(point.z, point.x);
//    float latitude = asin(point.y);
//    return vec2(longitude, latitude);
//}
//
//
//vec4 gridTexture(vec2 uv){
//    vec2 local = fract(uv*25.0);
//    int onborder= int(local.x<0.1||local.y<0.1);
//    return (1-onborder)*vec4(vec2(uv)/2.0+0.5, 0, 1) + onborder*vec4(1,1,1,1);
//}
//
//void main() {
//    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
//    vec2 uv = vec2(pixelCoord)/cubemap_res;
//
//    vec3 position = calcPosition(uv, tbn);
//    vec2 long_lat = longlat(position);
//    vec2 sampleUv = vec2(
//        (long_lat.x + PI) / (2.0 * PI),
//        (long_lat.y + PI / 2.0) / PI
//    );
//
//    float radius = cos(abs(long_lat.y));
//    float distortion = 1.0 / max(radius, 0.0001);
//    float level = log2(distortion);
//
//    float lowlevel = floor(level+1.0);
//    float local = abs(fract(level));
//
//
//
//    vec2 mappeduv = sampleUv*equirectTextureGridSize;
//
//    float index = floor(mappeduv.x) + floor(mappeduv.y)*equirectTextureGridSize.x;
//
//
//    vec3 color = texture(equirectTexture, vec3(mappeduv, index)).xyz;
//
//    imageStore(forward, pixelCoord, vec4(color, 1));
//}
//
//


#version 450 core

layout(binding = 0, rgba8) writeonly uniform image2D positive_x;
layout(binding = 1, rgba8) writeonly uniform image2D negative_x;
layout(binding = 2, rgba8) writeonly uniform image2D positive_y;
layout(binding = 3, rgba8) writeonly uniform image2D negative_y;
layout(binding = 4, rgba8) writeonly uniform image2D positive_z;
layout(binding = 5, rgba8) writeonly uniform image2D negative_z;



uniform sampler2DArray equirectTexture;

#define PI 3.1415926535

layout(local_size_x = 16, local_size_y = 16) in;


uniform mat3 tbn;
uniform vec2 equirectTextureGridSize;
uniform ivec2 cubemap_res;





vec3 calcPosition(vec2 uv, mat3 tbn){
    
    vec3 cubePos = tbn*vec3(uv*2-1, 1);
    return normalize(cubePos);
}
vec2 longlat(vec3 point) {

    float longitude = atan(point.z, point.x);
    float latitude = asin(point.y);
    return vec2(longitude, latitude);
}


vec4 gridTexture(vec2 uv){
    vec2 local = fract(uv*25.0);
    int onborder= int(local.x<0.1||local.y<0.1);
    return (1-onborder)*vec4(vec2(uv)/2.0+0.5, 0, 1) + onborder*vec4(1,1,1,1);
}

void main() {

    mat3 tbns[6] = {
        mat3(vec3(0,0, -1),vec3(0,-1,0),vec3(1,0,0)),
        mat3(vec3(0,0,1),vec3(0,-1,0),vec3(-1,0,0)),
        mat3(vec3(1,0,0),vec3(0,0,1),vec3(0,1,0)),
        mat3(vec3(1,0,0),vec3(0,0,-1),vec3(0,-1,0)),
        mat3(vec3(1,0,0),vec3(0,-1,0),vec3(0,0,1)),
        mat3(vec3(-1,0,0),vec3(0,-1,0),vec3(0,0,-1))
    };
     ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
     vec2 uv = vec2(pixelCoord)/cubemap_res;

    vec4 colors[6];
    for(uint i = 0;i<6;i++){
        

        vec3 position = calcPosition(uv, tbns[i]);
        vec2 long_lat = longlat(position);
        vec2 sampleUv = vec2(
            (long_lat.x + PI) / (2.0 * PI),
            (long_lat.y + PI / 2.0) / PI
        );

        float radius = cos(abs(long_lat.y));
        float distortion = 1.0 / max(radius, 0.0001);
        float level = log2(distortion);

        float lowlevel = floor(level+1.0);
        float local = abs(fract(level));


        vec2 mappeduv = sampleUv*equirectTextureGridSize;

        float index = floor(mappeduv.x) + floor(mappeduv.y)*equirectTextureGridSize.x;


        colors[i] = texture(equirectTexture, vec3(mappeduv, index));
    }

    imageStore(positive_x, pixelCoord, colors[0]);
   
    imageStore(negative_x, pixelCoord, colors[1]);
    
    imageStore(positive_y, pixelCoord, colors[2]);

    imageStore(negative_y, pixelCoord, colors[3]);

    imageStore(positive_z, pixelCoord, colors[4]);

    imageStore(negative_z, pixelCoord, colors[5]);

}


