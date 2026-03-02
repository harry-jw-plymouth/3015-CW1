#version 460

in vec3 Position;
in vec3 Normal;

in vec2 TexCoord;

layout (binding=0) uniform sampler2D MainTexture;
layout (binding=1) uniform sampler2D MixTexture;
layout (binding=2) uniform sampler2D;
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
    vec3 Ka; //how strongly the material reacts to ambient light
    vec3 Ks;
    float Shininess;
}Material;

//toon shading
const int Shaderlevels=6;
const float scaleFactor=1.0/Shaderlevels;

//s=light LightDirection, position=EyeCoordinates
vec3 BlinnphongSpot( vec3 position, vec3 n){
    vec3 diffuse=vec3(0), spec=vec3(0);
    vec3 texColor=texture(MainTexture,TexCoord).rgb;
    vec4 MixTexColour=texture(MixTexture,TexCoord);

    vec3 MixedColour=mix(texColor.rgb,MixTexColour.rgb,MixTexColour.a);
    
    vec3 AmbientLighting=Spot.La*Material.Ka*MixedColour;

    vec3 s=normalize(Spot.Position-position);

    float cosAng=dot(-s, normalize(Spot.Direction));
    float angle=acos(cosAng);
    float spotScale=0.0;

    if(angle>=0.0&&angle<Spot.Cutoff){
        spotScale=pow(cosAng,Spot.Exponent);
        float sDotN=max(dot(s,n),0.0);
        diffuse=Material.Kd*MixedColour*floor(sDotN*Shaderlevels)*scaleFactor;
        if(sDotN>0.0){
            vec3 v=normalize(-position.xyz);
            vec3 h=normalize(v+s);
            spec=Material.Ks*pow(max(dot(h,n),0.0),Material.Shininess);
        }
    }
    return AmbientLighting+spotScale*(diffuse+spec)*Spot.L;
}

void main() {


    FragColor=vec4(BlinnphongSpot(Position,normalize(Normal)),1.0);
}
