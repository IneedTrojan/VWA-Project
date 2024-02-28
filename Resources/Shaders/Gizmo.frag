#version 450
out
vec4 FragColor;


in VS_OUT {
    flat uint color;
} fs_in; 

vec4 unpackRGBA(uint packedColor) {
    float r = float((packedColor >> 24) & 0xFF) / 255.0;
    float g = float((packedColor >> 16) & 0xFF) / 255.0;
    float b = float((packedColor >> 8) & 0xFF) / 255.0;
    float a = float(packedColor & 0xFF) / 255.0;

    return vec4(r, g, b, a);
}

void main()
{
    FragColor = unpackRGBA(fs_in.color);
}