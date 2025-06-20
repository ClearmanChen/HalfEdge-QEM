#include "ObjLoader.h"


ObjLoader::ObjLoader()
{
	;
}

ObjLoader::~ObjLoader()
{
}

bool ObjLoader::load_file(std::string obj_path)
{
	if (obj_path.substr(obj_path.size() - 4, 4) != ".obj")
	{
		std::cerr << "ERROR::OBJ_LOADER::NOT_OBJ_FILE" << std::endl;
		return false;
	}
	
	std::ifstream file(obj_path);
	if (!file.is_open())
	{
		std::cerr << "ERROR::OBJ_LOADER::OBJ_FILE_CANNOT_OPEN" << std::endl;
		return false;
	}

	clear();
	std::string curline;
	while (std::getline(file, curline))
	{
		if (curline.size() <= 0)continue;
		auto elems = util::stringSplit(curline, " ");
		if (elems[0] == "v")
		{
			glm::vec3 vpos(
				std::stof(elems[1]),
				std::stof(elems[2]),
				std::stof(elems[3])
			);
			vertexes.push_back(GLVertex(vpos));
		}
		else if (elems[0] == "f")
		{
			glm::ivec3 ind(
				std::stoi(elems[1]) - 1,
				std::stoi(elems[2]) - 1,
				std::stoi(elems[3]) - 1
			);
			indices.push_back(ind);
		}
	}
}

void ObjLoader::clear()
{
	vertexes.clear();
	indices.clear();
}
