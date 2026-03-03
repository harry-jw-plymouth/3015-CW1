#version 460
//Colour value to send to next stage
out vec4 FragColor;

//Colour coordinates from last stage
in vec2 TexCoord;

uniform sampler2D GroundTexture;

void main()
{
    //Setting of colour coordinates to colour map
    FragColor = texture(GroundTexture,TexCoord);
}