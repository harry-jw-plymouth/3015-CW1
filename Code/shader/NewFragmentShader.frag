#version 460

in vec3 Position;
in vec3 Normal;
layout (location = 0)out vec4 FragColor;

uniform struct LightInfo{
    vec4 Position;
    vec3 La;
    vec3 L;
}lights[3];

uniform struct MaterialInfo{
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
}Material;

//s=light LightDirection, position=EyeCoordinates
vec3 phongModel(int light, vec3 EyeCoordinates, vec3 n){
    //calculate ambient here, to access each light LA value use this: lights[light].La 
    vec3 ambient=lights[light].La*Material.Ka;

    //calculate diffuse
    vec3 LightDirection=normalize(vec3(lights[light].Position.xyz)-EyeCoordinates);
    float sDotN=max(dot(LightDirection,n),0.0);
    vec3 diffuse = Material.Kd*sDotN;

    //calculate specular
    vec3 spec=vec3(0.0);
    if(sDotN>0.0){
        vec3 v=normalize(-EyeCoordinates.xyz);
        vec3 r =reflect(-LightDirection,n);
        spec=Material.Ks*pow(max(dot(r,v),0.0),Material.Shininess);
    }
    return ambient+lights[light].L*(diffuse+spec);
}

void main() {
    vec3 Colour=vec3(0.0);
    for(int i=0;i<3;i++){
        Colour+=phongModel(i,Position,Normal);
    }
    FragColor= vec4(Colour,1.0);  
}
