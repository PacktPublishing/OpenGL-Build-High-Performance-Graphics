#ifndef OBJ_INC
#define OBJ_INC
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp> 
#include <map>

using namespace std;

struct Vertex	{  
	glm::vec3 pos, 
			  normal;
	glm::vec2 uv;
}; 

struct Face { 
	unsigned short	a,b,c,  //pos indices
					d,e,f,  //normal indices
					g,h,i;  //uv indices
};

struct SubMesh {
	const char* materialName;
	vector<unsigned int> indices;
};
 

class EzmLoader {
	public :
		EzmLoader();
		~EzmLoader();

	bool Load(const string& filename, vector<SubMesh>& meshes, vector<Vertex>& verts, vector<unsigned short>& inds,	std::map<std::string, std::string>& materialNames, glm::vec3& min, glm::vec3& max);	
};
#endif
