#shader vertex

#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 atexCoord;

out vec3 outColor;
out vec2 texCoord;

uniform mat4 transform;

void main()
{
   gl_Position =  transform*vec4(position, 1.0);
   outColor = aColor;
   texCoord = vec2(atexCoord.x, atexCoord.y);
};

#shader fragment

#version 460 core

out vec4 fragColor;
in vec3 outColor;
in vec2 texCoord;

uniform sampler2D outTexture;

void main()
{
	fragColor = texture(outTexture, texCoord)*vec4(outColor, 1.0);

};