#pragma once
#ifndef CAM_H_
#define CAM_H_

#include "Window.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cam
{
public:
	// Default camera parameters
	glm::vec3 cam_pos;		// e  | Position of camera
	glm::vec3 cam_look_at;	// d  | This is where the camera looks at
	glm::vec3 cam_up;		// up | What orientation "up" is

	glm::mat4 toWorld;

	glm::vec3 cursorPos;
	glm::vec3 direction;


	Cam();
	~Cam();

	void trackball(glm::vec3);
	void moveVV(glm::vec3);
	void translate(int);
	void reset();

};
#endif
