#ifndef OBJ_INC
#define OBJ_INC
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //for matrices
#include <glm/gtx/quaternion.hpp>

#include <map>
#include "MeshImport.h"
using namespace std;

struct Vertex	{  
	glm::vec3 pos, 
			  normal;
	glm::vec2 uv;
	glm::vec4 blendWeights;
	glm::ivec4 blendIndices;
}; 

struct Bone {
	glm::quat orientation;
	glm::vec3 position;
	glm::mat4 xform, comb;
	glm::vec3 scale;
	char* name; 
	int parent;
};

struct SubMesh {
	const char* materialName;
	vector<unsigned int> indices;
};
 

class EzmLoader {
	public :
		EzmLoader();
		~EzmLoader();

	bool Load(const string& filename, vector<Bone>& skeleton, vector<NVSHARE::MeshAnimation>& animations, vector<SubMesh>& meshes, vector<Vertex>& verts, vector<unsigned short>& inds,	std::map<std::string, std::string>& materialNames, glm::vec3& min, glm::vec3& max);	
};
#endif
