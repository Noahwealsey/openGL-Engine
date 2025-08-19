#shader Vertex

#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    FragPos = vec3(u_model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_model))) * aNormal;
    TexCoords = aTexCoord;
    gl_Position = u_proj * u_view * vec4(FragPos, 1.0);
}

#shader Fragment

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
};
uniform Material material;

void main() {
    vec3 diffuse = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specular = texture(material.texture_specular1, TexCoords).rgb;
    FragColor = vec4(diffuse + specular, 1.0); // Simplified for debugging
}