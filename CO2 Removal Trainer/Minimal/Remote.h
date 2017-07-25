#ifndef REMOTES_H_
#define REMOTES_H_

#include <vector>
using namespace std;
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"

class Remote
{
public:

	GLuint VBO, VAO, EBO;
	glm::mat4 toWorld;

	glm::vec3 position;
	glm::mat4 quat;

	std::vector<glm::vec3> vertices;

	glm::vec3 colorVal;

	/*  Functions   */
	// Constructor, expects a filepath to a 3D model.
	Remote();
	vector<glm::vec3> calcCoords();
	// Draws the model, and thus all its meshes
	void Draw(GLuint shader);
};
#endif