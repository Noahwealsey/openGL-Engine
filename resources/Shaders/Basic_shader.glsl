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

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 objectColor;

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

void main()
{
    vec3 norm = normalize(Normal);
	vec3 lightDirNorm = normalize(-lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.);
	vec3 ambient = 0.01 * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 result = (ambient + diffuse) * objectColor;
	FragColor = vec4(result, 1.0);

}
