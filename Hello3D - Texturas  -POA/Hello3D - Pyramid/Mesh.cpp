#include "Mesh.h"

void Mesh::initialize(GLuint VAO, int nVertices, Shader* shader, GLuint texID, float ka, float kd, float ks)
{
	this->VAO = VAO;
	this->nVertices = nVertices;
	this->shader = shader;
	this->texID = texID;
	this->ka = ka;
	this->kd = kd;
	this->ks = ks;
	shader->setFloat("ka", ka);
	shader->setFloat("kd", kd);
	shader->setFloat("ks", ks);
}

void Mesh::update()
{
	
}

void Mesh::draw()
{
	glBindTexture(GL_TEXTURE_2D, texID);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, nVertices);
	glBindVertexArray(0);
}