#include "Lights.h"


Lights::Lights(int type) {
	mode = type;
	cursorPos = glm::vec3(0.0f, 0.0f, 0.0f);

	if (mode == 1) {//directional
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		direction = glm::vec3(-1.0f, 1.0f, 0.0f);//rays point in the x direction
		theta = 180.0f;

	}
	else if (mode == 2) {//point light
		position = glm::vec3(0.0f, 0.0f, 20.0f);
		direction = glm::vec3(0.0f, 0.0f, 0.0f);
		theta = 180.0f;
	}
	else if (mode == 3) {//spotlight
		position = glm::vec3(0.0f, 10.0f, 0.0f);
		direction = glm::vec3(0.0f, 1.0f, 0.0f);//rays point in the x direction
		theta = 30.0f;
	}
	
	cosExp = 2.0f;
	intensity = glm::vec3(1.0f, 1.0f, 1.0f);//white light
	ambient = 0.05f;
	specular = 0.5f;
	toWorld = glm::mat4(1.0f);
}
Lights::~Lights(){}

void Lights::trackball(glm::vec3 mPos)
{
	glm::vec3 dir = mPos - cursorPos; // current mouse position minus previous mouse position


	float bel = length(dir); // find length of curr - prev

	if (bel > 0.0001) { // if length of curr - prev is substantial
		glm::vec3 axisor; // axis of rotation
		axisor = glm::cross(cursorPos, mPos); //take cross product of prev and curr mouse positions
		float angleor = asin(bel / 2.0f) * 2.0f; // finds angle of rotation in radians
		angleor = (angleor * 180.0f) / 3.141592653f; //converts radians to degrees

		glLoadIdentity(); // load the identity matrix onto the matrix stack
		glRotatef(angleor, axisor.x, axisor.y, axisor.z); // rotate this identity matrix by angleor degrees around axisor
		//glMultMatrixf((GLfloat *)&toWorld[0][0]); // multiply this rotated matrix by the toWorld matrix
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)&toWorld[0][0]); // overwrite the toWorld matrix with this new matrix


		glm::vec4 deer(direction.x, direction.y, direction.z, 1.0f);
		glm::vec4 poos(position.x, position.y, position.z, 1.0f);
		deer = toWorld * deer;
		poos = toWorld * poos;
		deer = normalize(deer);

		direction = glm::vec3(deer.x, deer.y, deer.z);
		position = glm::vec3(poos.x, poos.y, poos.z);

		cursorPos = mPos; // set prev mouse position to new mouse position for next mouse movement

	}

}

void Lights::translate(float x, float y, float z) {

	/*float trans[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		x, y, z, 1.0f };*/


	//this->toWorld = glm::make_mat4(trans) * this->toWorld;

	glm::vec3 subber;

	if (position.x == 0.0f && position.y == 0.0f && position.z == 0.0f) 
		subber = glm::vec3(0.0f, 0.0f, 1.0f);
	else 
		subber = normalize(position);

	position = position + (subber * z);
	
}

void Lights::reset() {
	if (mode == 1) {//directional
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		direction = glm::vec3(1.0f, 0.0f, 0.0f);//rays point in the x direction
		theta = 180.0f;

	}
	else if (mode == 2) {//point light
		position = glm::vec3(0.0f, 0.0f, 20.0f);
		direction = glm::vec3(0.0f, 0.0f, 0.0f);
		theta = 180.0f;
	}
	else if (mode == 3) {//spotlight
		position = glm::vec3(0.0f, 10.0f, 0.0f);
		direction = glm::vec3(0.0f, 1.0f, 0.0f);//rays point in the x direction
		theta = 30.0f;
	}

	intensity = glm::vec3(1.0f, 1.0f, 1.0f);//white light
	ambient = 0.15f;
	specular = 0.5f;
	toWorld = glm::mat4(1.0f);
}

void Lights::spotWiden(float wide) {
	theta += (wide/4.0f);
	if (theta > 180.0f) {
		theta = 180.0f;
	}
	else if (theta < 0.0f) {
		theta = 0.0f;
	}
}