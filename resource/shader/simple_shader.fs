#version 330 core

uniform vec3 lightPos;
uniform vec3 diffuse;
uniform vec3 ambient;
uniform vec3 specular;
uniform vec3 viewPos;

in vec3 vertexColor;
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 specular = spec * specular * vec3(1.0f, 1.0f, 1.0f);
    vec3 diffuse = diff * vec3(1.0f, 1.0f, 1.0f);
    vec3 result = (ambient + diffuse + specular) * vertexColor;
    FragColor = vec4(result, 1.0f);
}