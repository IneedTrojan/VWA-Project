#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
    mat3 TBN;
    vec3 fragPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    vec3 worldPosition;
    vec2 uv;
} gs_in[];

out GS_OUT {
    mat3 TBN;
    vec3 fragPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    vec3 worldPosition;
    vec2 uv;
    flat float debug_area;
    vec3 triangleCoordinates;
} gs_out;

float calculateTriangleArea(vec2 v0, vec2 v1, vec2 v2) {
    return abs(0.5 * ((v0.x * (v1.y - v2.y)) + (v1.x * (v2.y - v0.y)) + (v2.x * (v0.y - v1.y))));
}

float inverseLerp(float min, float max, float val) {
    return clamp(0,1, (val - min) / (max - min));
}


uniform vec2 u_resolution;



void main() {
    
vec4 positions[3];
int visible = 0;

for (int i = 0; i < 3; i++) {
    vec4 ndcPos = gl_in[i].gl_Position / gl_in[i].gl_Position.w;
    vec2 adjustedPos = ndcPos.xy * 0.5 + 0.5;
    
    visible += int(adjustedPos.x > 0.0 && adjustedPos.x < 1.0 && adjustedPos.y > 0.0 && adjustedPos.y < 1.0);
    
    positions[i] = vec4(adjustedPos * u_resolution, 0, 1);
}

float area = calculateTriangleArea(positions[0].xy, positions[1].xy, positions[2].xy);

    for(int i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position;

        gs_out.triangleCoordinates = vec3(float(i==0), float(i==1), float(i==2));
        gs_out.debug_area = area;
        gs_out.TBN = gs_in[i].TBN;
        gs_out.fragPos = gs_in[i].fragPos;
        gs_out.tangentViewPos = gs_in[i].tangentViewPos;
        gs_out.tangentFragPos = gs_in[i].tangentFragPos;
        gs_out.worldPosition = gs_in[i].worldPosition;
        gs_out.uv = gs_in[i].uv;
        EmitVertex();
    }
    EndPrimitive();
}