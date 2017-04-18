#pragma once
#include <GL/freeglut.h>
#include <string.h>
#include <glm/glm.hpp>
#include <vector>

//our vertex struct stores the position and normals
struct Vertex {
	glm::vec3 pos, normal;
};

//TetrahedraMarcher class
class TetrahedraMarcher
{
public:
	//constructor/destructor
	TetrahedraMarcher(void);
	~TetrahedraMarcher(void);

	//function to set the volume dimension
	void SetVolumeDimensions(const int xdim, const int ydim, const int zdim);
	
	//function to set the total number of sampling voxels
	//more voxels will give a higher density mesh
	void SetNumSamplingVoxels(const int x, const int y, const int z);
	
	//set the isosurface value
	void SetIsosurfaceValue(const GLubyte value);
	
	//load the volume dataset
	bool LoadVolume(const std::string& filename);
	
	//march the volume dataset
	void MarchVolume();
	
	//get the total number of vertices generated
	size_t GetTotalVertices();
	
	//get the pointer to the vertex buffer
	Vertex* GetVertexPointer();

protected:
	//volume sampling function, give the x,y,z values returns the density value 
	//in the volume at that location
	GLubyte SampleVolume(const int x, const int y, const int z);
	
	//get the normal at the given location using center finite difference approximation
	glm::vec3 GetNormal(const int x, const int y, const int z);
	
	//samples a voxel at the given location and scale
	void SampleVoxel(const int x, const int y, const int z, glm::vec3 scale);
	
	//returns the offset between the two sample values
	float GetOffset(const GLubyte v1, const GLubyte v2);

	//the volume dataset dimensions and inverse volume dimensions
	int XDIM, YDIM, ZDIM;
	glm::vec3 invDim;

	//sampling distances in voxels
	int X_SAMPLING_DIST;
	int Y_SAMPLING_DIST;
	int Z_SAMPLING_DIST;

	//volume data pointer
	GLubyte* pVolume;
	
	//the given isovalue to look for
	GLubyte isoValue; 
	
	//vertices vector storing positions and normals
	std::vector<Vertex> vertices; 
};

