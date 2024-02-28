#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec4 debugColor;
    vec3 worldPosition;
} gs_in[];

out GS_OUT {
       vec4 debugColor;
     float score;
} gs_out;


void main() {
    /*
vec4 positions[3];

for (int i = 0; i < 3; i++) {
    positions[i] = vec4(gs_in[i].worldPosition, 0, 1);
}

float sideA = length(positions[1].xy - positions[0].xy);
float sideB = length(positions[2].xy - positions[1].xy);
float sideC = length(positions[0].xy - positions[2].xy);

float diffAB = abs(sideA - sideB);
float diffBC = abs(sideB - sideC);
float diffCA = abs(sideC - sideA);

float minDiff = min(diffAB, min(diffBC, diffCA));

float isocellnessScore;
if (minDiff == 0.0) {
    isocellnessScore = 1.0;
} else {
    isocellnessScore = 1.0 / (1.0 + minDiff);
}
*/
isocellnessScore = 1.0;


    for(int i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position;

        gs_out.debugColor = gl_in[i].debugColor;
        gs_out.score = isocellnessScore;

        EmitVertex();
    }
    EndPrimitive();
}