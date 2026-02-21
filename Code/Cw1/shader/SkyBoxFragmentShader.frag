#version 460

in vec3 Vec;
in vec3 LightDir;
in vec2 TexCoord;
in vec3 ViewDir;

layout(binding=0)uniform samplerCube SkyBoxTex; 

layout (location = 0)out vec4 FragColor;


void main() {
    vec3 texColor=texture(SkyBoxTex,normalize(Vec)).rgb;
    FragColor=vec4( texColor,0.0f);
}
