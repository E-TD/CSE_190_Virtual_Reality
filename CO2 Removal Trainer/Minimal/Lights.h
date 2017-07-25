#ifndef LIGHTS_H_
#define LIGHTS_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Lights
{
public:
	int mode;
	glm::vec3 direction;
	glm::vec3 position;
	glm::vec3 intensity;
	glm::vec3 cursorPos;

	float ambient;

	float specular;

	float theta;
	float cosExp;

	glm::mat4 toWorld;


	Lights(int);
	~Lights();

	void trackball(glm::vec3);
	void translate(float, float, float);
	void reset();
	void spotWiden(float);

};
#endif