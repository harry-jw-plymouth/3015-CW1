#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in vec3 LightDir;
in vec3 ViewDir;

layout (binding=0) uniform sampler2D StandardTexture;
layout(binding=1)uniform sampler2D NormalMapTex;


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


//toon shading values
const int levels=5;
const float scaleFactor=1.0/levels;

//s=light LightDirection, position=EyeCoordinates
vec3 BlinnphongSpot(  vec3 n){
    vec3 diffuse=vec3(0), spec=vec3(0);
    vec3 texColor=texture(StandardTexture,TexCoord).rgb;
    
   // vec3 ambient=Spot.La*Material.Ka*texColor;
   vec3 ambient=Spot.La*texColor;

    //vec3 s=normalize(Spot.Position-position);
    vec3 s=normalize(LightDir);

    float cosAng=dot(-s, normalize(Spot.Direction));
    float angle=acos(cosAng);
    float spotScale=0.0;

    if(angle>=0.0&&angle<Spot.Cutoff){
        spotScale=pow(cosAng,Spot.Exponent);
        float sDotN=max(dot(s,n),0.0);
        diffuse=Material.Kd*texColor*floor(levels* sDotN)*scaleFactor;
        if(sDotN>0.0){
          //  vec3 v=normalize(-position.xyz);
            vec3 v=normalize(ViewDir);
            vec3 h=normalize(v+s);
            spec=Material.Ks*pow(max(dot(h,n),0.0),Material.Shininess);
        }
    }
    return ambient+spotScale*(diffuse+spec)*Spot.L;
}

void main() {
    //do normal mapping
    vec3 Norm=texture(NormalMapTex,TexCoord).xyz;
    Norm.xy=2.0*Norm.xy-1.0;

    FragColor=vec4(BlinnphongSpot(normalize(Norm)),1.0);
  // FragColor=texture(Tex1,TexCoord);
}
