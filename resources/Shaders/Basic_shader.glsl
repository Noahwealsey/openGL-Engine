#shader Vertex

#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 u_modal;
uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
    FragPos = vec3(u_modal * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_modal))) * aNormal;
    gl_Position = u_proj*u_view*vec4(FragPos, 1.0);
}

#shader Fragment

#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;

uniform Material material;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 cameraPos;

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

void main()
{
    vec3 norm = normalize(Normal);

    // Calculate light direction (from fragment to light)
    vec3 lightDir = normalize(lightPos - FragPos);

    // Calculate view direction (from fragment to camera)
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // Ambient lighting
    vec3 ambient = light.ambient*material.ambient * lightColor;

    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse*diff * lightColor*material.diffuse;

    // Specular lighting
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular*material.specular * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
