#pragma once

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"


class Mesh
{
public:
	Mesh() {}
	~Mesh() {}
	void initialize(GLuint VAO, int nVertices, Shader* shader, GLuint texID, float ka, float kd, float ks);
	void update();
	void draw();
	Shader* shader;
protected:
	GLuint VAO; //Identificador do Vertex Array Object - V�rtices e seus atributos
	int nVertices;

	//Refer�ncia (endere�o) do shader
	
	GLuint texID; //id da textura
	float ka;
	float kd;
	float ks;
};

