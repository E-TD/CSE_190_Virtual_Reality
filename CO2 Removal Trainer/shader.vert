#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 fragVert;
out vec3 fragNormal;

void main()
{
	fragVert = vec3(model * vec4(position, 1.0f));
	mat4 modifier = transpose(inverse(model));
	vec4 thingy = modifier * vec4(normal, 1.0f);
	thingy = normalize(thingy);
	fragNormal = vec3(thingy);

    gl_Position = projection * view * model * vec4(position, 1.0f);
	//gl_Position = projection * modelView * vec4(position, 1.0f);
    TexCoords = texCoords;
}