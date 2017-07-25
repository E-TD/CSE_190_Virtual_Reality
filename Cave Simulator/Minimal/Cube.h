#pragma once


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

class Cube
{

public:
	glm::mat4 toWorld;
	Cube();
	float scaler;
	void draw(GLuint shaderProgram);
	void scale(float);
	void translateLEFTRIGHT(float);
	void translateUPDOWN(float);
	void translateBACKFORTH(float);
	void resetScale();
private:

	GLuint skyboxVAO, skyboxVBO;
	GLuint skyboxTexture;
	std::vector<const GLchar*> skybox_faces;


	GLuint loadCubemap(std::vector<const GLchar*> faces);
};