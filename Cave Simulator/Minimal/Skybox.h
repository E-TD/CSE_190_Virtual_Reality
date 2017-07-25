#pragma once


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

class Skybox
{

public:
	glm::mat4 toWorld;
	Skybox(bool);
	void draw(GLuint shaderProgram);
private:

	GLuint skyboxVAO, skyboxVBO;
	GLuint skyboxTexture;
	std::vector<const GLchar*> skybox_faces;

	unsigned char* loadPPM(const char* filename, int& width, int& height);
	GLuint loadCubemap(std::vector<const GLchar*> faces);
};