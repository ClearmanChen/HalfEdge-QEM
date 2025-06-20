#pragma once

#include "gl_header.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string>
#include "Util.h"
#include "primitives.h"

class ObjLoader
{
public:
	ObjLoader();
	~ObjLoader();

	bool load_file (std::string obj_path);
	// clear the stored data
	void clear();

	std::vector<GLVertex> vertexes;
	std::vector<glm::ivec3> indices;
};