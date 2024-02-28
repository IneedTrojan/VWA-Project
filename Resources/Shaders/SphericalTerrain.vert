#version 450 core

layout(location = 0) in
uint vertexID;

out VS_OUT {
    vec4 debugColor;
    vec3 worldPosition;
} vs_out;


uniform mat4 model;
uniform mat4 cameraMatrix;
uniform uint resolution;
uniform vec3 viewerPosition;

vec2 boundsOverflow(vec2 point, vec4 bounds){
    vec2 overflow;
    overflow.x = (point.x > bounds.x) ? ( point.x- bounds.x) : 
                  (point.x > bounds.z) ? (point.x - bounds.z) : 0.0;
    overflow.y = (point.y > bounds.y) ? (point.y - bounds.y) : 
                  (point.y > bounds.w) ? (point.y - bounds.w) : 0.0;
    return overflow;
}

vec3 rotateVectorAroundAxis(vec3 v, vec3 axis, float angle) {
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    return v * cosAngle + cross(axis, v) * sinAngle + axis * dot(axis, v) * (1.0 - cosAngle);
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

float hash(vec2 point){
    return sin(dot(point, vec2(20123.19823,1.23812)));
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
        total += noise3D(P * frequency) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity; // Double the frequency each octave
    }

    return total / maxAmplitude; // Normalize the result
}









void main()
{


    
    float PI = 3.1415926535897932384626433832795;
    uint px = vertexID % resolution;
    uint py = vertexID / resolution;

    float x = float(px) / float(resolution - 1);
    float y = float(py) / float(resolution - 1);
  
    float longitude = x * 2.0 * PI;

    float radius = 50000.0;
   
    float pi_interval = PI/float(resolution-1);

    float latitude = PI/2;

    for(int i = 0;i<py;i++){
        if(py == resolution-1){
            
            latitude = 0;
        }else{
            latitude -= asin(sin(latitude)*2.0*PI/float(resolution-1-float(i)/1.4));
        }
    }


    float px_world = radius * sin(latitude) * cos(longitude);
    float py_world = radius * sin(latitude) * sin(longitude);
    float pz_world = radius * cos(latitude);
    
    vec3 pos = vec3(px_world, py_world, pz_world);

    float d = pow(1.0-distance(pos, viewerPosition)/radius, 0.02);
    float h = fractionalNoise3D(pos/100, 10, 2.0,0.5);

    pos *= 1.0-(h)*0.005;


    pos.z -= radius;
    gl_Position = cameraMatrix * vec4(pos, 1.0);
    vs_out.debugColor = (vertexID%6<3)?vec4(0,0,0, 1.0): vec4(1);
}