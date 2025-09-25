#shader Vertex

#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
	gl_Position = u_proj * u_view * u_model * vec4(aPos, 1.0f);
}

#shader Fragment

#version 330 core

out vec4 FragColor;
uniform float u_time;


void main() {
    float t = (sin(u_time)); // Oscillate between 0 and 1
    vec3 color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), t);
    FragColor = vec4(color, 1.0);
}
