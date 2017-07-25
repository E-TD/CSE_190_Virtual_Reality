#version 330 core

// This is an example vertex shader. GLSL is very similar to C.
// You can define extra functions if needed, and the main() function is
// called when the vertex shader gets run.
// The vertex shader gets called once per vertex.

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 texCoords;

void main()
{
	mat4 VP = projection * view * model;
    gl_Position = VP * vec4(position, 1.0);
    texCoords = position;
}
