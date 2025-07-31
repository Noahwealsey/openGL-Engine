#shader Vertex

#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 u_modal;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    FragPos = vec3(u_modal * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_modal))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = u_proj * u_view * vec4(FragPos, 1.0);
}

#shader Fragment

#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};


struct Light {
    vec3 position;

    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 color;

    float constant;
	float linear;
	float quadratic;

	float cutoff; 
	float outerCutoff; 

    float exponent;

};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform Material material;
uniform Light light;

void main() {
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;
	float distance = length(light.position - FragPos);
	float attuenation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance* distance);
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
	vec3 spotDir = normalize(-light.direction);
    
	float theta = dot(lightDir, spotDir);
    float epsilon = light.cutoff - light.outerCutoff;
    float rawIntensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    // Apply smoothstep for more natural falloff
    float intensity = smoothstep(0.0, 1.0, rawIntensity);

    if (theta > light.cutoff) {
        float safeTheta = clamp(theta, 0.0, 1.0);
        intensity = pow(abs(safeTheta), light.exponent);
		FragColor = vec4(ambient * intensity, 1.0);
        
    }
    else {
        FragColor = vec4(ambient, 1.0);
	}
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;

    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;

	ambient *= attuenation;
	diffuse *= attuenation*intensity;
	specular *= attuenation*intensity;

    vec3 result = light.color*(ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
}

