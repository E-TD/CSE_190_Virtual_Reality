
#include "Window.h"
#include "Cube.h"

unsigned char* loadPPM(const char* filename, int& width, int& height)
{
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = (unsigned int)fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;
		return NULL;
	}

	return rawData;
}

Cube::Cube()
{
	toWorld = glm::mat4(1.0f);
	scaler = 10.0f;
	float scaleVal = 0.01f;

	GLfloat skyboxVertices[] = {
		// Positions          
		-scaleVal,  scaleVal, -scaleVal,
		-scaleVal, -scaleVal, -scaleVal,
		scaleVal, -scaleVal, -scaleVal,
		scaleVal, -scaleVal, -scaleVal,
		scaleVal,  scaleVal, -scaleVal,
		-scaleVal,  scaleVal, -scaleVal,

		-scaleVal, -scaleVal,  scaleVal,
		-scaleVal, -scaleVal, -scaleVal,
		-scaleVal,  scaleVal, -scaleVal,
		-scaleVal,  scaleVal, -scaleVal,
		-scaleVal,  scaleVal,  scaleVal,
		-scaleVal, -scaleVal,  scaleVal,

		scaleVal, -scaleVal, -scaleVal,
		scaleVal, -scaleVal,  scaleVal,
		scaleVal,  scaleVal,  scaleVal,
		scaleVal,  scaleVal,  scaleVal,
		scaleVal,  scaleVal, -scaleVal,
		scaleVal, -scaleVal, -scaleVal,

		-scaleVal, -scaleVal,  scaleVal,
		-scaleVal,  scaleVal,  scaleVal,
		scaleVal,  scaleVal,  scaleVal,
		scaleVal,  scaleVal,  scaleVal,
		scaleVal, -scaleVal,  scaleVal,
		-scaleVal, -scaleVal,  scaleVal,

		-scaleVal,  scaleVal, -scaleVal,
		scaleVal,  scaleVal, -scaleVal,
		scaleVal,  scaleVal,  scaleVal,
		scaleVal,  scaleVal,  scaleVal,
		-scaleVal,  scaleVal,  scaleVal,
		-scaleVal,  scaleVal, -scaleVal,

		-scaleVal, -scaleVal, -scaleVal,
		-scaleVal, -scaleVal,  scaleVal,
		scaleVal, -scaleVal, -scaleVal,
		scaleVal, -scaleVal, -scaleVal,
		-scaleVal, -scaleVal,  scaleVal,
		scaleVal, -scaleVal,  scaleVal
	};

	//list of names for skybox
	skybox_faces.push_back("vr_test_pattern.ppm");
	skybox_faces.push_back("vr_test_pattern.ppm");
	skybox_faces.push_back("vr_test_pattern.ppm");
	skybox_faces.push_back("vr_test_pattern.ppm");
	skybox_faces.push_back("vr_test_pattern.ppm");
	skybox_faces.push_back("vr_test_pattern.ppm");
	// Setup skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
	skyboxTexture = loadCubemap(skybox_faces);
}

GLuint Cube::loadCubemap(std::vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = loadPPM(faces[i], width, height);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
		);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// Generate mipmaps, by the way.
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

void Cube::draw(GLuint shaderProgram)
{
	GLuint uModel = glGetUniformLocation(shaderProgram, "model");
	glm::mat4 model = glm::translate(toWorld, glm::vec3(0.0f, 0.0f, -0.3f));
	model = glm::scale(model, glm::vec3(scaler, scaler, scaler));

	glUniformMatrix4fv(uModel, 1, GL_FALSE, &model[0][0]);


	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
}

void Cube::scale(float scaleVal) {
	scaler += scaleVal;
	if (scaler < 1.0f) scaler = 1.0f;
	if (scaler > 100.0f) scaler = 100.0f;
}

void Cube::resetScale() {
	scaler = 10.0f;
}

void Cube::translateLEFTRIGHT(float transVal) {
	glm::vec3 temp = glm::vec3(transVal, 0.0f, 0.0f);
	toWorld = glm::translate(toWorld, temp);
}

void Cube::translateUPDOWN(float transVal) {
	glm::vec3 temp = glm::vec3(0.0f, transVal, 0.0f);
	toWorld = glm::translate(toWorld, temp);
}

void Cube::translateBACKFORTH(float transVal) {
	glm::vec3 temp = glm::vec3(0.0f, 0.0f, transVal);
	toWorld = glm::translate(toWorld, temp);
}