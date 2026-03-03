#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;
layout(location = 3) in vec4 VertexTangent;

out vec3 LightDir;
out vec3 ViewDir;
out vec2 TexCoord;

uniform struct LightInfo{
    vec4 position;
    vec3 La;
    vec3 L;
}Light;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

void main()
{
    //Transform normal and tangent to eye space 
    vec3 norm=normalize(NormalMatrix*VertexNormal);
    vec3 tang=normalize(NormalMatrix*vec3(VertexTangent));
    //compute the binormal
    vec3 binormal=normalize(cross(norm,tang))*VertexTangent.w;
    //Matrix for transformation to tangent space
    vec3 Position=(ModelViewMatrix*vec4(VertexPosition,1.0)).xyz;

    mat3 toObjectLocal=mat3(
        tang.x,binormal.x,norm.x,
        tang.y,binormal.y,norm.y,
        tang.z,binormal.z,norm.z
    );
   
   
    LightDir=toObjectLocal*(Light.position.xyz-Position);
    ViewDir=toObjectLocal*normalize(-Position);
    TexCoord =VertexTexCoord;
    
    gl_Position=MVP*vec4(VertexPosition,1.0);
}
