#include "Material.h"

void Material::initialize(string file, string name, float ka, float kd, float ks)
{
	this->file = file;
	this->name = name;
	this->ka = ka;
	this->kd = kd;
	this->ks = ks;
}