#version 450

layout(local_size_x = 512, local_size_y = 1, local_size_z = 1) in;



layout(r16, binding = 1) uniform image2DArray heightmap; 



uniform ivec3 resolution;
uniform double radius;
uniform double multiplier;



int BufferFetchIndex(ivec3 pixelCoords, ivec3 bufferSize){
    int i = pixelCoords.x +
    pixelCoords.y * bufferSize.x +
    pixelCoords.z * bufferSize.x*bufferSize.y;
    return i;
}

vec4 unpackRGBA(int color){
    return vec4((color<<0)&255,(color<<8)&255,(color<<16)&255,(color<<24)&255)/255;
}

struct Task{
    dmat4 model;

    ivec2 pixelMin;
    ivec2 pixelMax;
    ivec2 cameraPixelPosition;
    int layer;
    int exponent;
};


layout(std430, binding = 1) buffer Tasks {
    Task tasks[];
};

ivec2 Overflow(ivec2 point, ivec4 bounds) {
    return ivec2(
        (point.x - bounds.x) * int(point.x < bounds.x) + (point.x - bounds.z) * int(point.x > bounds.z),
        (point.y - bounds.y) * int(point.y < bounds.y) + (point.y - bounds.w) * int(point.y > bounds.w)
    );
}



int intFract(int value, int modulus) {
    int remainder = value % modulus;
    remainder += int(remainder < 0) * modulus;
    return remainder;
}

ivec2 intFract2(ivec2 value, int modulus){
    return ivec2(intFract(value.x, modulus), intFract(value.y, modulus));
}




vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x * 34.0) + 1.0) * x);
}

vec4 taylorInvSqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}
vec3 fade(vec3 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
vec4 fade(vec4 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

const mat2 myt = mat2(.12121212, .13131313, -.13131313, .12121212);
const vec2 mys = vec2(1e4, 1e6);

vec2 rhash(vec2 uv) {
  uv *= myt;
  uv *= mys;
  return fract(fract(uv / mys) * uv);
}

vec3 hash(vec3 p) {
  return fract(
      sin(vec3(dot(p, vec3(1.0, 57.0, 113.0)), dot(p, vec3(57.0, 113.0, 1.0)),
               dot(p, vec3(113.0, 1.0, 57.0)))) *
      43758.5453);
}

vec3 voronoi3d(const in vec3 x) {
  vec3 p = floor(x);
  vec3 f = fract(x);

  float id = 0.0;
  vec2 res = vec2(100.0);
  for (int k = -1; k <= 1; k++) {
    for (int j = -1; j <= 1; j++) {
      for (int i = -1; i <= 1; i++) {
        vec3 b = vec3(float(i), float(j), float(k));
        vec3 r = vec3(b) - f + hash(p + b);
        float d = dot(r, r);

        float cond = max(sign(res.x - d), 0.0);
        float nCond = 1.0 - cond;

        float cond2 = nCond * max(sign(res.y - d), 0.0);
        float nCond2 = 1.0 - cond2;

        id = (dot(p + b, vec3(1.0, 57.0, 113.0)) * cond) + (id * nCond);
        res = vec2(d, res.x) * cond + res * nCond;

        res.y = cond2 * d + nCond2 * res.y;
      }
    }
  }

  return vec3(sqrt(res), abs(id));
}



float noise3D(vec3 P) {
    vec4 Pi0 = floor(vec4(P, P.y + P.z + P.x)); // Integer part, xyzw
    vec4 Pi1 = Pi0 + 1.0; // Integer part + 1, xyzw
    Pi0 = mod289(Pi0);
    Pi1 = mod289(Pi1);
    vec4 Pf0 = fract(vec4(P, P.y + P.z + P.x)); // Fractional part, xyzw
    vec4 Pf1 = Pf0 - 1.0; // Fractional part - 1.0, xyzw

    vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    vec4 iy = vec4(Pi0.yy, Pi1.yy);
    vec4 iz0 = Pi0.zzzz;
    vec4 iz1 = Pi1.zzzz;

    vec4 ixy = permute(permute(ix) + iy);
    vec4 ixy0 = permute(ixy + iz0);
    vec4 ixy1 = permute(ixy + iz1);

    vec4 gx0 = ixy0 * (1.0 / 7.0);
    vec4 gy0 = floor(gx0) * (1.0 / 7.0);
    vec4 gz0 = floor(gy0) * (1.0 / 6.0);
    gx0 = fract(gx0) - 0.5;
    gy0 = fract(gy0) - 0.5;
    gz0 = fract(gz0) - 0.5;
    vec4 gw0 = vec4(0.75) - abs(gx0) - abs(gy0) - abs(gz0);
    vec4 sw0 = step(gw0, vec4(0.0));
    gx0 -= sw0 * (step(0.0, gx0) - 0.5);
    gy0 -= sw0 * (step(0.0, gy0) - 0.5);

    vec4 gx1 = ixy1 * (1.0 / 7.0);
    vec4 gy1 = floor(gx1) * (1.0 / 7.0);
    vec4 gz1 = floor(gy1) * (1.0 / 6.0);
    gx1 = fract(gx1) - 0.5;
    gy1 = fract(gy1) - 0.5;
    gz1 = fract(gz1) - 0.5;
    vec4 gw1 = vec4(0.75) - abs(gx1) - abs(gy1) - abs(gz1);
    vec4 sw1 = step(gw1, vec4(0.0));
    gx1 -= sw1 * (step(0.0, gx1) - 0.5);
    gy1 -= sw1 * (step(0.0, gy1) - 0.5);

    vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
    vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
    vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
    vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
    vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
    vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
    vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
    vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

    vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
    g000 *= norm0.x;
    g010 *= norm0.y;
    g100 *= norm0.z;
    g110 *= norm0.w;
    vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
    g001 *= norm1.x;
    g011 *= norm1.y;
    g101 *= norm1.z;
    g111 *= norm1.w;

    float n000 = dot(g000, Pf0.xyz);
    float n001 = dot(g001, Pf0.xyz - vec3(0, 0, 1));
    float n010 = dot(g010, Pf0.xyz - vec3(0, 1, 0));
    float n011 = dot(g011, Pf0.xyz - vec3(0, 1, 1));
    float n100 = dot(g100, Pf0.xyz - vec3(1, 0, 0));
    float n101 = dot(g101, Pf0.xyz - vec3(1, 0, 1));
    float n110 = dot(g110, Pf0.xyz - vec3(1, 1, 0));
    float n111 = dot(g111, Pf0.xyz - vec3(1, 1, 1));

    vec3 fade_xyz = fade(Pf0.xyz);
    vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
    vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
    float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
    return 2.2 * n_xyz;
}




float fractionalNoise3D(vec3 P, int octaves, float lacunarity, float persistence) {
    float total = 0.0;
    float maxAmplitude = 0.0;
    float amplitude = 1;
    float frequency = 1;

    for(int i = 0; i < octaves; i++) {
        total += (noise3D(P * frequency)*voronoi3d(P * frequency).x) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity; // Double the frequency each octave
    }

    return (total / maxAmplitude)/2.0+0.5; // Normalize the result
}


vec2 uvInBounds(ivec2 point, ivec4 bounds) {
    float u = float(point.x - bounds.x) / float(bounds.z - bounds.x);
    float v = float(point.y - bounds.y) / float(bounds.w - bounds.y);
    return vec2(u, v);
}
dvec3 cubeSphereProjection(dvec3 worldPosition)
	{
		dvec3 squared = dvec3( worldPosition.x * worldPosition.x, worldPosition.y * worldPosition.y, worldPosition.z * worldPosition.z);

		double x = worldPosition.x * sqrt(1.0 - (squared.y - squared.z) / 2.0 + (squared.y * squared.z) / 3.0);
		double y = worldPosition.y * sqrt(1.0 - (squared.z - squared.x) / 2.0 + (squared.z * squared.x) / 3.0);
		double z = worldPosition.z * sqrt(1.0 - (squared.x - squared.y) / 2.0 + (squared.x * squared.y) / 3.0);


		return dvec3(x, y, z);
	}

void main() {

    int thread_count_x = 512;

    int id = int(gl_GlobalInvocationID.x);
    int localId = id % int(resolution.x);
    int taskIndex = id / thread_count_x;
     Task task = tasks[taskIndex];
    int layer = task.layer;
    int res = resolution.x;


    ivec2 pixelMin = task.pixelMin;
    ivec2 pixelMax = task.pixelMax;
    ivec2 cameraPixelPosition = task.cameraPixelPosition;


    ivec2 inTexCoord = pixelMin + (pixelMax - pixelMin) * localId / res;

    ivec2 true_pixel_pos = inTexCoord + cameraPixelPosition;

    
    ivec2 texCoord2D = intFract2(true_pixel_pos, res);

    dvec3 flatPosition = (task.model*dvec4(dvec2(inTexCoord), 0, 1)).xyz;

    flatPosition = cubeSphereProjection(flatPosition/radius)*radius;


    float depth = fractionalNoise3D(vec3(flatPosition/radius), 12, 2.0,0.5);

    int index = layer * res*res+
    texCoord2D.x + (texCoord2D.y * res);



    imageStore(heightmap, ivec3(texCoord2D, layer), vec4(depth.x,0,0,0));

}
