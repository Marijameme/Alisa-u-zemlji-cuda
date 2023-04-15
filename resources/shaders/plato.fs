#version 330 core
out vec4 FragColor;

struct Material{
    sampler2D diffuse;
    sampler2D specular;
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
uniform Material m;
uniform PointLight l;

void main()
{
    //ambient
    vec4 ambient = texture(m.diffuse, TexCoords) * vec4(l.ambient, 1.0f);

    //diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(l.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = texture(m.diffuse, TexCoords) * diff * vec4(l.diffuse, 1.0f);

    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectionDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), m.shininess);
    vec4 specular = (texture(m.specular, TexCoords) * spec) * vec4(l.specular, 1.0f);

    //result
    float distance = length(l.position - FragPos);
    float attenuation = 1.0 / (l.constant + l.linear * distance + l.quadratic * (distance * distance));
    vec4 result = attenuation * (ambient + diffuse + specular);
    //there is no point in implementing blending 'cause the grass texture doesn't have clear areas
    if(result.a < 0.55)
        discard;
	FragColor = result;
}