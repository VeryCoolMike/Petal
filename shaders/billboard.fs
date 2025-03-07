#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D currentTexture;
uniform vec3 colour;
void main()
{
   FragColor = texture(currentTexture, TexCoord) * vec4(colour, 1.0);
};