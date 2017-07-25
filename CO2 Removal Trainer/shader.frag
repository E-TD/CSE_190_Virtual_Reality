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

	//this block calculates the diffuse color
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(light.direction);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuseColor = diff * light.intensity;
	diffuseColor.x = diffuseColor.x * material.diffuse.x;
	diffuseColor.y = diffuseColor.y * material.diffuse.y;
	diffuseColor.z = diffuseColor.z * material.diffuse.z;

	//this block calculates ambient color
	vec3 ambColor = light.ambient * light.intensity;
	ambColor.x = ambColor.x * material.ambient.x;
	ambColor.y = ambColor.y * material.ambient.y;
	ambColor.z = ambColor.z * material.ambient.z;

	//this block calculates the spec color
	vec3 viewDir = normalize(cameraPos - fragVert);
	vec3 reflectDir = reflect(-lightDir, norm);
	float specul = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	//vec3 specColor = light.spec * specul * light.intensity * material.shininess;
	vec3 specColor = vec3(0.0f,0.0f,0.0f);

	vec3 tempCo = (ambColor + diffuseColor + specColor);
	//tempCo.x = tempCo.x * material.objColor.x;
	//tempCo.y = tempCo.y * material.objColor.y;
	//tempCo.z = tempCo.z * material.objColor.z;

	color = vec4(tempCo, 1.0f);



    //color = vec4(diffuseColor, 1.0f);
}