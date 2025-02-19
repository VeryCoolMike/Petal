#version 400 core
out vec4 FragColor;

struct PointLight
{
    vec3 position;
    vec3 color;
    bool enabled;
    bool castShadow;
    //int lightType;
    // 0 = Directional light
    // 1 = Point light
    // 2 = Spot light
    int shadowID;
    float strength;
};


in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

#define MAX_LIGHTS 500
#define MAX_SHADOWS 6

uniform samplerCube shadowMap[MAX_SHADOWS];
uniform samplerCube dynamicShadowMap[MAX_SHADOWS];
uniform vec3 lightPos[MAX_SHADOWS];

uniform sampler2D currentTexture;
uniform samplerCube skybox;
uniform sampler2D depthTexture;
uniform sampler2D normalTexture;
uniform vec3 objectColor;
uniform float ambientStrength;
uniform vec3 viewPos;
uniform bool selected;
uniform float reflectancy;
uniform float far_plane;
uniform bool shadowsEnabled;
uniform bool shadowDebug;

uniform int lightAmount;
uniform PointLight pointLights[MAX_LIGHTS];



float ShadowCalculation(vec3 fragPos, int id)
{

    vec3 fragToLight = fragPos - lightPos[id];
    float currentDepth = length(fragToLight);
    if (currentDepth >= far_plane)
    {
        return 2.0;
    }

    float bias = 0.15f;
    float viewDistance = length(viewPos - fragPos);

    float staticDepth = texture(shadowMap[id], fragToLight).r;
    staticDepth *= far_plane;

    float dynamicDepth = texture(dynamicShadowMap[id], fragToLight).r;
    dynamicDepth *= far_plane;

    float shadowStatic = (currentDepth - bias > staticDepth) ? 1.0 : 0.0;
    float shadowDynamic = (currentDepth - bias > dynamicDepth) ? 1.0 : 0.0;

    float shadow = max(shadowStatic, shadowDynamic);

    if (shadowDebug == true)
    {
        FragColor = vec4(vec3(shadowStatic), 1.0);
    }
    

    return shadow;
}

vec3 calcPointLight(PointLight light)
{
    mediump float distance = length(light.position - fs_in.FragPos);
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    
    float shininess = 32.0;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * light.color;
    
    vec3 ambient = ambientStrength * light.color;
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color;
    
    mediump float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = 0.0f;

    if (shadowsEnabled == true)
    {
        if (light.castShadow == true)
        {
            shadow = ShadowCalculation(fs_in.FragPos, light.shadowID);
        }
    }
    
    return (ambient + (1.0 - shadow) * (diffuse + specular)) * objectColor;
}

void main()
{
    vec3 result = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < lightAmount; i++)
    {
        if (pointLights[i].enabled == true)
        {
            result += calcPointLight(pointLights[i]) * pointLights[i].strength;
        }
    }


    if (shadowDebug == false)
    {
        if (reflectancy > 0.0)
        {
            vec3 I = normalize(fs_in.FragPos - viewPos) * reflectancy;
            vec3 R = reflect(I, normalize(fs_in.Normal));

            FragColor = (texture(currentTexture, fs_in.TexCoord) * vec4(result, 1.0)) * (vec4(texture(skybox, R).rgb, 1.0f));
        }
        else
        {
            FragColor = (texture(currentTexture, fs_in.TexCoord) * vec4(result, 1.0));
        }
    }
    
    vec2 screenTexCoord = gl_FragCoord.xy / vec2(1920, 1080);
    /*
    float depth = texture(depthTexture, screenTexCoord).r;
    float near = 0.1;
    float far = 100.0;
    float viewSpaceDepth = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
    FragColor = vec4(vec3(viewSpaceDepth), 1.0);
    */
    //vec3 normal = normalize(texture(normalTexture, screenTexCoord).xyz);
    //FragColor = vec4(vec3(normal), 1.0);




    if (selected == true)
    {
        FragColor = texture(currentTexture, fs_in.TexCoord);
    }
}