#pragma once

#include <vector>
#include <string>
#include <sstream>

using namespace std;

#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/types.h>
#include <assimp/scene.h>


struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};


struct Texture {
	GLuint id;
	string type;
	aiString path;
};

struct Material {
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
};

class Mesh {


public:

	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);
	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, aiMaterial* material);


	/*  Mesh Data  */
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;
	Material material;
	/*  Functions  */
	void Draw(GLuint shaderId);
private:
	/*  Render data  */
	GLuint VAO, VBO, EBO;
	/*  Functions    */
	void setupMesh();
};