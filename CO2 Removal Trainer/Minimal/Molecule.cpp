#include "Molecule.h"
#include "Window.h"

 // generates a random float between a and b
float rf(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

Molecule::Molecule(Model* model)
{
	this->model = model;
	float disp = -50.0f;
	float range = 30.0f;

	// initialize toWorld Matrix with random initial location
	this->toWorld = glm::mat4(1.0f);
	this->center = glm::vec3(rf(-range, range), rf(-range, range), rf(-range + disp, range + disp));

	// initialize random velocity vector
	this->velocity = glm::vec3(rf(-0.25, 0.25), rf(-0.25, 0.25), rf(-0.25, 0.25));

	// initialize random rotation vector
	this->spinner = glm::normalize(glm::vec3(rf(-1.0, 1.0), rf(-1.0, 1.0), rf(-1.0, 1.0)));

}

// Draws the model, and thus all its meshes
void Molecule::Draw(GLuint shaderProgram)
{
	this->model->toWorld = glm::translate(glm::mat4(1.0f), this->center) * this->toWorld;
	this->model->Draw(shaderProgram);
}

void Molecule::update(bool endstate) {
	float disp = -50.0f;
	float range = 30.0f;
	if (endstate)
		range = 100.0f;
	this->center = this->center + this->velocity;
	if (this->center[0] > range || this->center[0] < -range) {
		this->velocity[0] = -this->velocity[0];
	}
	if (this->center[1] > range || this->center[1] < -range) {
		this->velocity[1] = -this->velocity[1];
	}
	if (this->center[2] > range + disp || this->center[2] < -range + disp) {
		this->velocity[2] = -this->velocity[2];
	}

	toWorld = toWorld * glm::rotate(glm::mat4(1.0f), 5.0f / 180.0f * glm::pi<float>(), this->spinner);
}