#include "Remote.h"

Remote::Remote() {
	colorVal = glm::vec3(0.0f, 1.0f, 0.0f);

	toWorld = glm::mat4(1.0f);

	vertices.clear();

	vertices.push_back(glm::vec3(0.0f,0.0f, -5000.0f));
	vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);

	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive vertex attributes. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glEnableVertexAttribArray(0); //enable the var

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

}

vector<glm::vec3> Remote::calcCoords() {
	vector<glm::vec3> toRet;
	glm::vec4 zeroes(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 fixMat = glm::translate(quat, position);

	glm::vec4 temp = fixMat * zeroes;
	glm::vec3 toPush(temp[0], temp[1], temp[2]);
	toRet.push_back(toPush);

	glm::vec4 maxZ = glm::vec4(0.0f, 0.0f, -5000.0f, 1.0f);
	temp = fixMat * maxZ;
	toPush = glm::vec3(temp[0], temp[1], temp[2]);
	toRet.push_back(toPush);

	return toRet;
}

void Remote::Draw(GLuint shaderProgram) {

	toWorld = glm::translate(quat, position);

	glLineWidth(10.0f);
	// draw lines
	GLuint MatrixID = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &toWorld[0][0]);

	GLuint colorID = glGetUniformLocation(shaderProgram, "colorVal");
	glUniform3f(colorID, colorVal.x, colorVal.y, colorVal.z);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}
