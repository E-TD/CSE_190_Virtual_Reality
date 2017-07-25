#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragVert;

void main()
{
	fragVert = vec3(model * vec4(position, 1.0f));
	//mat4 modifier = transpose(inverse(model));
	//vec4 thingy = modifier * vec4(normal, 1.0f);
	//thingy = normalize(thingy);
	//fragNormal = vec3(thingy);

    gl_Position = projection * view * model * vec4(position, 1.0f);
}