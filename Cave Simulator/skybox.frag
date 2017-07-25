#version 330 core

// This is a sample fragment shader.

in vec3 texCoords;

out vec4 color;

uniform samplerCube skybox;

void main()
{
	//color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	color = texture(skybox, texCoords);
	
}
