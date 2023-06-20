#pragma once

#include <vector>
#include <stb_image.h>

#include "Mesh.h"
#include "Material.h"
#include "Bezier.h"

using namespace std;

class Object
{
public:
	Object() {}
	void initialize(string folder, string obj, string mtl, Shader *shader, glm::vec3 position = glm::vec3(0.0, 0.0, 0.0), glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0), float angle = 0.0, glm::vec3 axis = glm::vec3(0.0, 0.0, 1.0), bool selected = false);
	void initalizeAnimation(bool animated, Bezier bezier);
	void update();
	void draw();

	//Informações sobre as transformações a serem aplicadas no objeto
	glm::vec3 position;
	glm::vec3 scale;
	float angle;
	glm::vec3 axis;
	vector <Mesh> grupos;
	Shader* shader;
	bool selected;
	bool animated;
	Bezier bezier;
	int animationIndex;
	int increment;
protected:
	void loadObj(string folder, string obj, string mtl);
	GLuint generateVAO(vector<GLfloat> vertbuffer, int& nVerts);
	GLuint loadTexture(string filePath);
};

