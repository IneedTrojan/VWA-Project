#version 450 core

in VS_OUT {
    smooth vec3 normalizedPosition;
} fs_in;

out vec4 fragColor;

uniform samplerCube skyboxTexture;

void main(){
  
    fragColor = texture(skyboxTexture, fs_in.normalizedPosition);
   
}