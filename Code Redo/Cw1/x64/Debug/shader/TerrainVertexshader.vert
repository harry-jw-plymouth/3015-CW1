#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 colourVertex;

//Model-View-Projection Matrix
uniform mat4 mvpIn;
uniform vec2 TerrainStart;
uniform float TerrainSize;

out vec2 TexCoord;

void main()
{
    //Transformation applied to vertices
    gl_Position = mvpIn * vec4(position.x, position.y, position.z, 1.0);
    //Sending texture coordinates to next stage
    TexCoord=(position.xz-TerrainStart)/TerrainSize;
}