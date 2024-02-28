#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 uv;
    vec3 worldPosition;
    vec4 debugColor;
    vec3 normal;
} gs_in[];

out GS_OUT {
     vec2 uv;
     vec3 worldPosition;
     vec4 debugColor;
     vec3 normal;
     vec3 lerp;
     float debug_area;
} gs_out;


float calculateTriangleArea(vec2 v0, vec2 v1, vec2 v2) {
    return abs(0.5 * ((v0.x * (v1.y - v2.y)) + (v1.x * (v2.y - v0.y)) + (v2.x * (v0.y - v1.y))));
}

void main() {
    vec4 positions[3];

    vec2 u_resolution = vec2(1024.0, 1024.0);

    for (int i = 0; i < 3; i++) {
        positions[i] = gl_in[i].gl_Position / gl_in[i].gl_Position.w;
        // Convert from NDC to screen space if necessary
        positions[i].xy = positions[i].xy * 0.5 + 0.5;
        positions[i].xy *= u_resolution;
    }

    float area = calculateTriangleArea(positions[0].xy, positions[1].xy, positions[2].xy);

    for(int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;

        gs_out.lerp = vec3(float(i == 0), float(i == 1), float(i == 2));
        gs_out.debug_area = area;
        gs_out.worldPosition = gs_in[i].worldPosition;
        gs_out.uv = gs_in[i].uv;
        gs_out.debugColor = gs_in[i].debugColor;
        gs_out.normal = gs_in[i].normal;
        EmitVertex();
    }
    EndPrimitive();
}
