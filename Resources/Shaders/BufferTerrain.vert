#version 450


uniform float radius;
uniform vec3 cameraPosition;
uniform float elevation;
uniform mat4 cameraMatrix;
uniform ivec2 resolution;
uniform sampler2DArray heightmapTexture;

struct Level{
    mat4 model;
    ivec4 activeIndices;
    ivec4 scrollxscrollyfacetextureLayer;

};



layout(location = 0) in uint vertexID;//-0.5,0.5

out VS_OUT {
    vec2 uv;
    vec3 worldPosition;
    vec4 debugColor;
    vec3 normal;
} vs_out;


layout(std430, binding = 4)buffer LevelSSBO{
    Level levels[];
};

int BufferFetchIndex(ivec3 pixelCoords, ivec3 bufferSize){
    return pixelCoords.x +
    pixelCoords.y * bufferSize.x +
    pixelCoords.z * bufferSize.x*bufferSize.y;
}

//
//struct Camera {
//    mat4 modelSpaceMatrix;
//    mat4 viewSpaceMatrix;
//    mat4 camSpaceMatrix;
//};
//
//
//
//layout(std430, binding = 0) buffer CameraSSBO {
//    Camera cameraMatrices[];
//};
//

float lerp(float _min, float _max, float _l){
    return _min+(_max-_min)*_l;
}
float inverseLerp(float _min, float _max, float _val){
    return (_val - _min) / (_max - _min);
}
int intFract(int value, int modulus) {
    int remainder = value % modulus;
    remainder += int(remainder < 0) * modulus;
    return remainder;
}

ivec2 intFract2(ivec2 value, int modulus){
    return ivec2(intFract(value.x, modulus), intFract(value.y, modulus));
}

bool is_in_clip(vec4 pos) {
    vec3 ndc = pos.xyz / pos.w;

    return all(lessThanEqual(ndc, vec3(1.0))) && all(greaterThanEqual(ndc, vec3(-1.0)));
}

dvec3 cubeSphereProjection(dvec3 worldPosition)
	{
        return normalize(vec3(worldPosition));
		dvec3 squared = dvec3( worldPosition.x * worldPosition.x, worldPosition.y * worldPosition.y, worldPosition.z * worldPosition.z);

		double x = worldPosition.x * sqrt(float(1.0 - (squared.y + squared.z) / 2.0 + (squared.y * squared.z) / 3.0));
		double y = worldPosition.y * sqrt(float(1.0 - (squared.z + squared.x) / 2.0 + (squared.z * squared.x) / 3.0));
		double z = worldPosition.z * sqrt(float(1.0 - (squared.x + squared.y) / 2.0 + (squared.x * squared.y) / 3.0));


		return dvec3(x, y, z);
	}


int fetchLocation(ivec2 localPosition, ivec2 cameraPixelPosition){
    
    ivec2 scroll = cameraPixelPosition;
    scroll += ivec2(localPosition);

    scroll = intFract2(scroll, resolution.x);
    int fetchIndex = (scroll.x%resolution.x) + (scroll.y*resolution.x);
    return fetchIndex;
}





bool isInRange(float minimum , float maximum, float v){
    return minimum <= v && v <= maximum;
}

bool inBoundingBox(ivec2 p, ivec4 bounds) {
    return p.x >= bounds.x && p.x <= bounds.z &&
           p.y >= bounds.y && p.y <= bounds.w;
}



mat3 normalizeMatrix(mat3 m) {
    // Normalize the first column vector
    vec3 c1 = normalize(m[0]);

    // Orthogonalize the second column vector to the first
    vec3 c2 = normalize(m[1] - dot(c1, m[1]) * c1);

    // Compute the third column as the cross product of the first two to ensure orthogonality
    vec3 c3 = cross(c1, c2);

    // Construct and return the normalized matrix
    return mat3(c1, c2, c3);
}

vec3 terrainColor(vec3 normal, float height){

const int heightSize = 4; // Number of height categories
    const int normalSize = 4; // Number of normal categories
    vec4 colors[heightSize][normalSize] = {
    // Low elevation: From water to lush flatlands
    {
        vec4(0.0, 0.3, 0.7, 1.0), // Deep water
        vec4(0.0, 0.45, 0.75, 1.0), // Shallow water
        vec4(0.2, 0.6, 0.2, 1.0), // Flat grass
        vec4(0.25, 0.55, 0.2, 1.0) // Steep grass
    },
    // Mid elevation: From grassland to rocky terrain
    {
        vec4(0.3, 0.7, 0.3, 1.0), // Lush grassland
        vec4(0.5, 0.5, 0.0, 1.0), // Transition zone
        vec4(0.6, 0.5, 0.3, 1.0), // Flat rocky
        vec4(0.5, 0.4, 0.3, 1.0) // Steep rocky
    },
    // High elevation: From rocky terrain to snowline
    {
        vec4(0.6, 0.6, 0.6, 1.0), // Light rock
        vec4(0.7, 0.7, 0.7, 1.0), // Dark rock
        vec4(0.9, 0.9, 0.9, 1.0), // Snow transition
        vec4(1.0, 1.0, 1.0, 1.0) // Snow
    },
    // Very high elevation: Snow-covered peaks with varying steepness
    {
        vec4(0.9, 0.9, 0.9, 1.0), // Flat snow
        vec4(1.0, 1.0, 1.0, 1.0), // Slightly steep snow
        vec4(0.95, 0.95, 1.0, 1.0), // Very steep snow
        vec4(0.9, 0.9, 1.0, 1.0) // Cliffs
    }
};

   // Scale and compute indices for height
    float heightIndex = floor(height * float(heightSize - 1));
    float heightBlend = fract(height * float(heightSize - 1));

    // Assuming normal.y is normalized between 0 and 1, scale for normal variation
    float normalIndex = floor(normal.y * float(normalSize - 1));
    float normalBlend = fract(normal.y * float(normalSize - 1));

    // Bilinear interpolation
    vec4 bottomLeft = colors[int(heightIndex)][int(normalIndex)];
    vec4 bottomRight = colors[int(heightIndex)][clamp(int(normalIndex) + 1, 0, normalSize - 1)];
    vec4 topLeft = colors[clamp(int(heightIndex) + 1, 0, heightSize - 1)][int(normalIndex)];
    vec4 topRight = colors[clamp(int(heightIndex) + 1, 0, heightSize - 1)][clamp(int(normalIndex) + 1, 0, normalSize - 1)];

    vec4 bottomBlend = mix(bottomLeft, bottomRight, normalBlend);
    vec4 topBlend = mix(topLeft, topRight, normalBlend);
    vec4 finalColor = mix(bottomBlend, topBlend, heightBlend);

    return vec3(finalColor);
}


void main() {

    int res = resolution.x;
    float size = float(1<<gl_InstanceID);
    float normalscale = elevation*res;
     float quarter = res/4;
    int _half = res/2;

    Level level = levels[gl_InstanceID];
    mat4 model = level.model;
    mat4 clip = cameraMatrix;
    ivec4 bounds = level.activeIndices;
    ivec2 scroll = level.scrollxscrollyfacetextureLayer.xy;
    int face = level.scrollxscrollyfacetextureLayer.z;
    int textureLayer = level.scrollxscrollyfacetextureLayer.w;

    ivec2 localPosition = ivec2((vertexID%resolution.x), (vertexID/resolution.x));
    vec3 worldPosition = (model*vec4(localPosition, 0, 1)).xyz;

    worldPosition = vec3(cubeSphereProjection(worldPosition/radius))*radius;

    vec3 sphereNormal = -normalize(worldPosition);
   
 
    bool culled = inBoundingBox(localPosition, bounds);

    
    float drop = float(culled);

    int overshoot = 7;
    int undershoot = 7;

    ivec2 pixelPosNormal = intFract2(localPosition + scroll + -ivec2(undershoot,undershoot), res);
    ivec2 pixelPosTangent = intFract2(localPosition + scroll + ivec2(overshoot, 0), res);
    ivec2 pixelPosBitangent = intFract2(localPosition + scroll + ivec2(0, overshoot), res);


   vec3 sampleNormal = texelFetch(heightmapTexture, ivec3(pixelPosNormal, textureLayer), 0).xyz;
   vec3 sampleTangent = texelFetch(heightmapTexture, ivec3(pixelPosTangent, textureLayer), 0).xyz;
   vec3 sampleBitangent = texelFetch(heightmapTexture, ivec3(pixelPosBitangent, textureLayer), 0).xyz;

    vec3 blendColor = (sampleNormal+sampleTangent+sampleBitangent)/3.0;

    float height = sampleNormal.x;
    float right = sampleTangent.x;
    float down = sampleBitangent.x; 
    
    height = (height+right+down)/3.0 ;

    vec3 vectorRight = vec3(1.0, (right - height)*normalscale/size, 0.0);
    vec3 vectorDown = vec3(0.0, (down - height)*normalscale/size, 1.0);

    vec3 normal = cross(vectorRight, vectorDown);

    normal = normalize(normal);

    

    vec3 arbitraryVector = vec3(0.0, 1.0, 0.0);
    if (abs(dot(sphereNormal, arbitraryVector)) > 0.999)
        arbitraryVector = vec3(1.0, 0.0, 0.0);


    vec3 tangent = normalize(cross(sphereNormal, arbitraryVector));

    vec3 bitangent = normalize(cross(sphereNormal, tangent));

    mat3 newTBN = mat3(tangent, bitangent, sphereNormal);

    worldPosition *= (height*1.0+0.9- drop);
    



    vec4 screen_space_pos = clip*vec4(worldPosition, 1.0);
    vs_out.worldPosition = worldPosition;
    vs_out.uv = vec2(0);
    vs_out.debugColor = vec4(terrainColor(normal, height), 1)*1.2;
    vs_out.normal = vec3(0,0,0);
    gl_Position = screen_space_pos;
   
    

   }