#shader Vertex

#version 330 core
layout(location = 0) in vec3 lightPos;

uniform mat4 u_modal;
uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
	gl_Position = u_proj* u_view*u_modal*vec4(lightPos, 1.0f);
}

#shader Fragment

#version 330 core

out vec4 FragColor;

void main(){
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}

