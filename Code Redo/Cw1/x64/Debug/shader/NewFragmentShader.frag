#version 460

in vec3 LightDir;
in vec2 TexCoord;
in vec3 ViewDir;

layout(binding=0) uniform sampler2D ColorTex;
layout(binding=1) uniform sampler2D NormalMapTex;

layout (location = 0)out vec4 FragColor;

uniform struct SpotLightInfo{
    vec3 Position;
    vec3 La;
    vec3 L;
    vec3 Direction;
    float Exponent;
    float Cutoff;
}Spot;

uniform struct MaterialInfo{
    vec3 Kd;
    vec3 Ka; //how strongly the ,aterial reacts to ambient light
    vec3 Ks;
    float Shininess;
}Material;

//s=light LightDirection, position=EyeCoordinates
vec3 BlinnphongSpot(vec3 n){
    vec3 diffuse=vec3(0), spec=vec3(0);

    vec3 texColor=texture(ColorTex,TexCoord).rgb;
    
    vec3 ambient=Spot.La*texColor;
    vec3 s=normalize(LightDir);
    float sDotN=max(dot(s,n),0.0);

    diffuse=texColor*sDotN;
    if(sDotN>0.0){
        vec3 v=normalize(ViewDir);
        vec3 h=normalize(v+s);
        spec=Material.Ks*pow(max(dot(h,n),0.0),Material.Shininess);
    }
    return ambient+(diffuse+spec)*Spot.L;
}

void main() {
    vec3 Norm=texture(NormalMapTex,TexCoord).xyz;
    Norm.xy=2.0*Norm.xy-1.0;

    FragColor=vec4(BlinnphongSpot(normalize(Norm)),1.0);

}
