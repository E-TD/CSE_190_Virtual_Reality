#version 330 core

in vec3 fragVert;
in vec3 fragNormal;
in vec2 TexCoords;

out vec4 color;

struct Light {
	vec3 intensity;
	vec3 direction;
	float ambient;

	float theta;
	float cosExp;

	vec3 pos;
	float spec;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

uniform Material material;
uniform Light light;
uniform vec3 cameraPos;
uniform sampler2D texture_diffuse1;

void main()
{
	color = texture(texture_diffuse1, TexCoords);
}