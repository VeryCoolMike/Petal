#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gMaterial;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform float reflectancy;

uniform sampler2D albedoTexture;

void main()
{    
    gPosition = vec4(FragPos, 1.0);
    gNormal = vec4(normalize(Normal), 1.0);
    gAlbedo = texture(albedoTexture, TexCoord) * vec4(objectColor, 1.0);
    gMaterial = vec4(reflectancy, 0.0, 0.0, 1.0);
}