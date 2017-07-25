#version 330 core

// This is a sample fragment shader.

in vec2 texCoord;

out vec4 color;

uniform sampler2D caveTex;

void main()
{
	//color = vec4(0.0f, texCoord, 1.0f);
	color = texture(caveTex, texCoord);
	
}
