#version 330 core
out vec4 FragColor;

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform Material m;
uniform Light l;

void main()
{
    //ambient
    vec3 ambient = texture(m.diffuse, TexCoords).rgb * l.ambient;

    //diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(l.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = texture(m.diffuse, TexCoords).rgb * diff * l.diffuse;

    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectionDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), m.shininess);
    vec3 specular = (texture(m.specular, TexCoords).rgb * spec) * l.specular;

    //result
    vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}