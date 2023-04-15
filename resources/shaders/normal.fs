#version 330 core
out vec4 FragColor;
out vec4 BrightColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;


struct PointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};


uniform vec3 viewPos;
uniform Material material;

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{

        vec3 norm;
        norm = texture(material.texture_normal1, TexCoords).rgb;
        norm = norm * 2.0 - 1.0;
        norm = normalize(TBN * norm);



    vec3 viewDir = normalize(viewPos - FragPos);

    vec4 result = vec4(0.f, 0.f, 0.f, 1.f);//CalcDirLight(dirLight, norm, viewDir);

//     for(int i = 0; i < NR_POINT_LIGHTS; i++)
//     {
//             result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
//     }
//
//     result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    FragColor = result;

     float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
        if(brightness > 1.0)
            BrightColor = vec4(FragColor.rgb, 1.0);
        else
            BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

}

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    vec3 halfwayDir = normalize(lightDir + viewDir);

    vec4 ambient = vec4(light.ambient, 1.f) * texture(material.texture_diffuse1, TexCoords);

    vec4 diffuse = vec4(light.diffuse, 1.f) * max(dot(lightDir, normal), 0.0) * texture(material.texture_diffuse1, TexCoords);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec4 specular = vec4(light.specular, 1.f) * pow(max(dot(normal, halfwayDir), 0.0), material.shininess) * texture(material.texture_specular1, TexCoords);

    float distance = length(fragPos - light.position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}


