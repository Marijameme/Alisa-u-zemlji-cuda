#version 330 core
out vec4 FragColor;

struct Material{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};

struct PointLight {
    vec3 position;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform PointLight l;
uniform Material material;

void main()
{
    //ambient
    vec4 ambient = texture(material.texture_diffuse1, TexCoords) * vec4(l.ambient, 1.0f);

    //diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(l.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = texture(material.texture_diffuse1, TexCoords) * diff * vec4(l.diffuse, 1.0f);

    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectionDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec4 specular = (texture(material.texture_specular1, TexCoords) * spec) * vec4(l.specular, 1.0f);

    //result
    float distance = length(l.position - FragPos);
    float attenuation = 1.0 / (l.constant + l.linear * distance + l.quadratic * (distance * distance));
    vec4 result = attenuation * (ambient + diffuse + specular);
    if(result.a < 0.1)
            discard;
    FragColor = result;
}