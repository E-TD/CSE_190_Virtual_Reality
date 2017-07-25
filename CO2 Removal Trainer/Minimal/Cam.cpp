#include "Cam.h"


Cam::Cam() {
	// Default camera parameters
	cam_pos = glm::vec3(0.0f, 0.0f, 20.0f);		// e  | Position of camera
	cam_look_at = glm::vec3(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
	cam_up = glm::vec3(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is
	toWorld = glm::mat4(1.0f);
	direction = cam_look_at - cam_pos;
	direction = normalize(direction);
}

Cam::~Cam() {}

void Cam::moveVV(glm::vec3 currPos) {

	glm::vec3 dir = - currPos + cursorPos; // current mouse position minus previous mouse position
	glm::vec3 curruntPos(currPos.x, currPos.y, currPos.z);
	glm::vec3 zplane(0.0f, 0.0f, -1.0f);
	//std::cout << "currPos: "<< currPos.x << currPos.y << currPos.z << std::endl;
	//std::cout << "cursorPos: " << cursorPos.x << cursorPos.y << cursorPos.z << std::endl;

	float bel = length(dir); // find length of curr - prev

	if (bel > 0.0001) { // if length of curr - prev is substantial
		glm::vec4 direct(direction, 1.0f);
		float rotAngle = 0.0f;

		// find rotation angle in y axis
		rotAngle = asin(dir.y / 2.0f) * 2.0f;
		rotAngle = (rotAngle * 180.0f) / 3.141592653f; //converts radians to degrees
		glm::vec3 crossr = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction);
		glLoadIdentity(); // load the identity matrix onto the matrix stack
		glRotatef(rotAngle, crossr.x, crossr.y, crossr.z); // rotate this identity matrix by angleor degrees around axisor
														   //glMultMatrixf((GLfloat *)&toWorld[0][0]); // multiply this rotated matrix by the toWorld matrix
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)&toWorld[0][0]); // overwrite the toWorld matrix with this new matrix
		direct = toWorld * direct; // translate camera look at val by rotAngle degrees about the y axis

		// find angle of rotation in x axis
		rotAngle = asin(dir.x / 2.0f) * 2.0f;
		rotAngle = (rotAngle * 180.0f) / 3.141592653f; //converts radians to degrees
		glLoadIdentity(); // load the identity matrix onto the matrix stack
		glRotatef(rotAngle, 0.0f, 1.0f, 0.0f); // rotate this identity matrix by angleor degrees around axisor
											   //glMultMatrixf((GLfloat *)&toWorld[0][0]); // multiply this rotated matrix by the toWorld matrix
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)&toWorld[0][0]); // overwrite the toWorld matrix with this new matrix
		direct = toWorld * direct; // translate direct (which was previously translated) by x degrees


		direction = glm::vec3(direct.x, direct.y, direct.z);
		cam_look_at = cam_pos + direction;
	}
	cursorPos = curruntPos; // set prev mouse position to new mouse position for next mouse movement
}


void Cam::translate(int ver) {

	if (ver == 0) {//"W"		
		cam_pos = cam_pos + (direction);
		cam_look_at = cam_look_at + (direction);
	}
	else if (ver == 1) {//"A"
						//std::cout << "A\n";
		glm::vec3 du = glm::cross(cam_up, direction);
		cam_pos = cam_pos + (du);
		cam_look_at = cam_look_at + (du);
	}
	else if (ver == 2) {//"s"
						//std::cout << "S\n";
		cam_pos = cam_pos - (direction);
		cam_look_at = cam_look_at - (direction);
	}
	else if (ver == 3) {//"D"
						//std::cout << "D\n";
		glm::vec3 du = glm::cross(cam_up, direction);
		cam_pos = cam_pos - (du);
		cam_look_at = cam_look_at - (du);
	}

}

void Cam::reset() {
	// Default camera parameters
	cam_pos = glm::vec3(0.0f, 0.0f, 20.0f);		// e  | Position of camera
	cam_look_at = glm::vec3(0.0f, 0.0f, 19.0f);	// d  | This is where the camera looks at
	cam_up = glm::vec3(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is
	toWorld = glm::mat4(1.0f);
	direction = cam_look_at - cam_pos;
	direction = normalize(direction);
}
