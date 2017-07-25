#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>

#include "glm/gtx/string_cast.hpp" // print vec3 to string

#include <vector>

#include <windows.h>

class Face {
public:
	GLuint VAO, VBO;
	GLuint texture;
	std::vector<GLfloat> vertices;

	static unsigned char* loadaPPM(const char* filename, int& width, int& height)
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
			delete[] rawData;
			width = 0;
			height = 0;
			return NULL;
		}

		return rawData;
	}

	Face(std::vector<GLfloat> verts) {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Setup filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		int width, height;
		unsigned char* image = loadaPPM("vr_test_pattern.ppm", width, height);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, image);

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		vertices = verts;

		// Setup VAO
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
		glBindVertexArray(0);
	}
	void draw(GLuint shaderProgram, GLuint tex) {
		//glBindTexture(GL_TEXTURE_2D, texture);
		glBindTexture(GL_TEXTURE_2D, tex);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

	}

};

class Cave
{

public:
	glm::mat4 toWorld;
	Cave();
	void draw(GLuint shaderProgram, GLuint * tex);
private:

	std::vector<Face*> faces;
};