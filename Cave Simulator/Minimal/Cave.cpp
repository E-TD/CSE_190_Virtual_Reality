
#include "Cave.h"

Cave::Cave()
{
	toWorld = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f),glm::vec3(0.0f,1.0f,0.0f));

	float scaleVal = 1.2f;

	std::vector<GLfloat> right({
		scaleVal, -scaleVal, -scaleVal, 0.0f, 0.0f,
		scaleVal, -scaleVal,  scaleVal, 1.0f, 0.0f,
		scaleVal,  scaleVal,  scaleVal, 1.0f, 1.0f,
		scaleVal,  scaleVal,  scaleVal, 1.0f, 1.0f,
		scaleVal,  scaleVal, -scaleVal, 0.0f, 1.0f,
		scaleVal, -scaleVal, -scaleVal, 0.0f, 0.0f
	});

	std::vector<GLfloat> front({
		-scaleVal, -scaleVal,  -scaleVal, 0.0f, 0.0f,
		-scaleVal,  scaleVal,  -scaleVal, 0.0f, 1.0f,
		scaleVal,  scaleVal,  -scaleVal, 1.0f, 1.0f,
		scaleVal,  scaleVal,  -scaleVal, 1.0f, 1.0f,
		scaleVal, -scaleVal,  -scaleVal, 1.0f, 0.0f,
		-scaleVal, -scaleVal,  -scaleVal, 0.0f, 0.0f
	});

	std::vector<GLfloat> bottom({
		-scaleVal, -scaleVal, -scaleVal, 0.0f, 1.0f,
		-scaleVal, -scaleVal,  scaleVal, 0.0f, 0.0f,
		scaleVal, -scaleVal, -scaleVal, 1.0f, 1.0f,
		scaleVal, -scaleVal, -scaleVal, 1.0f, 1.0f,
		-scaleVal, -scaleVal,  scaleVal, 0.0f, 0.0f,
		scaleVal, -scaleVal,  scaleVal, 1.0f, 0.0f
	});

	faces.push_back(new Face(front));
	faces.push_back(new Face(right));
	faces.push_back(new Face(bottom));

}

void Cave::draw(GLuint shaderProgram, GLuint * tex)
{
	GLuint uModel = glGetUniformLocation(shaderProgram, "model");

	glUniformMatrix4fv(uModel, 1, GL_FALSE, &toWorld[0][0]);

	for (int i = 0; i < faces.size(); i++) {
		faces[i]->draw(shaderProgram, tex[i]);
	}
	
}

