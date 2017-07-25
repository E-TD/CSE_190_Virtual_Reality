#include "window.h"


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <ctime>

const char* window_title = "CSE190 Project 1";
Cam * playerCam;
Lights* light;

GLint shaderProgram;

// Default camera parameters

glm::vec3 cam_pos(0.0f, 0.0f, 20.0f);		// e  | Position of camera
glm::vec3 cam_look_at(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

int Window::width;
int Window::height;

bool cameraMode = false;
clock_t begin_time;

glm::mat4 Window::P;
glm::mat4 Window::V;

using namespace std;

Model* factory;
Model* co2;
Model* o2;
vector<Molecule*> co2_mols; // vector of all co2 molecules in the scene
vector<Molecule*> o2_mols; // vector of all o2 molecules in the scene

bool endState = false;

void Window::initialize_objects()
{
	playerCam = new Cam();
	light = new Lights(1); // creates a directional light pointing sideways

	// Load the shader program. Make sure you have the correct filepath up top

	factory = new Model("../models/factory2/factory2.obj");
	co2 = new Model("../models/co2/co2.obj");
	Molecule* temp;

	// adds 5 molecules with random displacement to the scene
	glm::vec3 rand_displacement;
	for (int i = 0; i < 5; i++) {
		temp = new Molecule(co2);
		co2_mols.push_back(temp);
	}

	shaderProgram = LoadShaders("../shader.vert", "../shader.frag");
	begin_time = clock();

}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::clean_up()
{
	delete(playerCam);
	delete(light);
	delete(factory);
	for (int i = 0; i < co2_mols.size(); i++) {
		delete(co2_mols[i]);
	}
	glDeleteProgram(shaderProgram);
}

GLFWwindow* Window::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Window::resize_callback(window, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);

	}
}

void Window::idle_callback()
{
	if (!endState) {
		clock_t end_time = clock();
		if (double(end_time - begin_time) / CLOCKS_PER_SEC > 1.0) {
			co2_mols.push_back(new Molecule(co2));

			if (co2_mols.size() > 10) {
				endState = true;
				for (int i = 0; i < 300; i++) {
					co2_mols.push_back(new Molecule(co2));
				}
			}

			begin_time = clock();
		}
	}
	// draw all CO2 molecules
	for (int i = 0; i < co2_mols.size(); i++) {
		co2_mols[i]->update();
	}
}

void Window::display_callback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_TRUE);

	// Use the shader of programID
	glUseProgram(shaderProgram);
	V = glm::lookAt(playerCam->cam_pos, playerCam->cam_look_at, playerCam->cam_up);

	// lighting block
	GLuint MatrixID = glGetUniformLocation(shaderProgram, "light.intensity");
	glUniform3f(MatrixID, (light->intensity.x), (light->intensity.y), (light->intensity.z));

	MatrixID = glGetUniformLocation(shaderProgram, "light.direction");
	glUniform3f(MatrixID, (light->direction.x), (light->direction.y), (light->direction.z));

	MatrixID = glGetUniformLocation(shaderProgram, "light.ambient");
	glUniform1f(MatrixID, light->ambient);

	MatrixID = glGetUniformLocation(shaderProgram, "light.pos");
	glUniform3f(MatrixID, light->position.x, light->position.y, light->position.z);

	MatrixID = glGetUniformLocation(shaderProgram, "light.spec");
	glUniform1f(MatrixID, light->specular);

	MatrixID = glGetUniformLocation(shaderProgram, "light.theta");
	glUniform1f(MatrixID, light->theta);

	MatrixID = glGetUniformLocation(shaderProgram, "light.cosExp");
	glUniform1f(MatrixID, light->cosExp);

	MatrixID = glGetUniformLocation(shaderProgram, "cameraPos");
	glUniform3f(MatrixID, (playerCam->cam_pos.x), (playerCam->cam_pos.y), (playerCam->cam_pos.z));

	factory->Draw(shaderProgram); // Draw the factory

	// draw all active molecules
	for (int i = 0; i < co2_mols.size(); i++) {
		co2_mols[i]->Draw(shaderProgram);
	}

	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

void Window::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	glm::vec3 mousePos;
	float d;

	if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) && !glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) && cameraMode)
	{
		// manipulate the camera
		mousePos.x = ((2.0f * (float)xpos) - Window::width) / Window::width;
		mousePos.y = (Window::height - (2.0f * (float)ypos)) / Window::height;
		mousePos.z = 0.0f;
		d = length(mousePos);
		d = (d < 1.0f) ? d : 1.0f;

		playerCam->moveVV(mousePos);
		V = glm::lookAt(playerCam->cam_pos, playerCam->cam_look_at, playerCam->cam_up);
	}

}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		// Check if escape was pressed
		if (key == GLFW_KEY_GRAVE_ACCENT)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (key == GLFW_KEY_ESCAPE) { // `
			cameraMode = !cameraMode;

			if (cameraMode) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
		else if (key == GLFW_KEY_W)
		{
			playerCam->translate(0);
		}
		else if (key == GLFW_KEY_A)
		{
			playerCam->translate(1);
		}
		else if (key == GLFW_KEY_S)
		{
			playerCam->translate(2);
		}
		else if (key == GLFW_KEY_D)
		{
			playerCam->translate(3);
		}
	}
	else if (action == GLFW_REPEAT) {
		if (key == GLFW_KEY_W)
		{
			playerCam->translate(0);
		}
		else if (key == GLFW_KEY_A)
		{
			playerCam->translate(1);
		}
		else if (key == GLFW_KEY_S)
		{
			playerCam->translate(2);
		}
		else if (key == GLFW_KEY_D)
		{
			playerCam->translate(3);
		}
	}
	V = glm::lookAt(playerCam->cam_pos, playerCam->cam_look_at, playerCam->cam_up);

}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

		glm::vec3 tempor = normalize(playerCam->cam_pos);

		playerCam->cam_pos = playerCam->cam_pos - ((float)(yoffset)*tempor);
		V = glm::lookAt(playerCam->cam_pos, playerCam->cam_look_at, playerCam->cam_up);
	
}