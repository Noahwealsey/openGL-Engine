#shader Vertex

#version 330 core
layout(location = 0) in vec3 lightPos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
	gl_Position = u_proj* u_view*u_model*vec4(lightPos, 1.0f);
}

#shader Fragment

#version 330 core

uniform vec3 lightColor;

out vec4 FragColor;

void main(){
	FragColor = vec4(lightColor, 1.0);
}

