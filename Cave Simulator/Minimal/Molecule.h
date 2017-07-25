#ifndef MOLECULE_H_
#define MOLECULE_H_

#include <vector>
using namespace std;
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"

class Molecule
{
public:
	Model* model;
	glm::mat4 toWorld;

	glm::vec3 center;
	glm::vec3 velocity;
	glm::vec3 spinner;
	/*  Functions   */
	// Constructor, expects a filepath to a 3D model.
	Molecule(Model* model);
	// Draws the model, and thus all its meshes
	void Draw(GLuint shader);
	void update(bool endstate);
};
#endif