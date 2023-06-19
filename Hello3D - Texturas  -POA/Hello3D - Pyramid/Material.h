#pragma once
#include <vector>
#include <stb_image.h>

#include "Mesh.h"

using namespace std;

class Material
{
public:
	Material() {}
	void initialize(string file, string name, float ka, float kd, float ks);

	string file;
	string name;
	float ka;
	float kd;
	float ks;
	
};

