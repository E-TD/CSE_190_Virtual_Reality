#version 330 core

in vec3 fragVert;

uniform vec3 colorVal;

out vec4 color;

void main()
{
    //color = vec4(0.0f,1.0f,0.0f,0.0f);
	color = vec4(colorVal, 0.0f);
}