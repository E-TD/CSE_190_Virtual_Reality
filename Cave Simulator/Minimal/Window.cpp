#include "Window.h"


GLint Window::shaderProgram;
GLint skyboxShader;
GLint caveShader;
GLuint Window::lineShader;
Model* Window::factory;

vector<Remote*> Window::remotes;
Remote* remote;

Skybox* Window::skybox;

Cube* Window::cube;
Cave* Window::cave;

void Window::initialize(ovrSession& _session) {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	ovr_RecenterTrackingOrigin(_session);

	remote = new Remote();
	factory = new Model("../models/cube.obj");
	factory->toWorld = glm::scale(factory->toWorld, glm::vec3(5.0f,5.0f,5.0f));
	factory->toWorld = glm::translate(factory->toWorld, glm::vec3(0.0f, 0.0f, -5.0f));

	cube = new Cube();
	cave = new Cave();

	lineShader = LoadShaders("../trackshader.vert", "../trackshader.frag");
	skyboxShader = LoadShaders("../skybox.vert", "../skybox.frag");
	caveShader = LoadShaders("../caveShader.vert", "../caveShader.frag");
	shaderProgram = caveShader;
}

void Window::reset(ovrSession& _session) {

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	ovr_RecenterTrackingOrigin(_session);
}

void Window::displayCallback(const glm::mat4 & projection, const glm::mat4 & headPose) {


	glUseProgram(skyboxShader);
	GLuint uProjection = glGetUniformLocation(skyboxShader, "projection");
	GLuint uModelView = glGetUniformLocation(skyboxShader, "view");

	glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, glm::value_ptr(glm::inverse(headPose)));
	skybox->draw(skyboxShader);
	cube->draw(skyboxShader);

}