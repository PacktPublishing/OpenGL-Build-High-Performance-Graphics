#include "VolumeSplatter.h"
#include "Tables.h"

#include <fstream> 

VolumeSplatter::VolumeSplatter(void)
{
	XDIM = 256;
	YDIM = 256;
	ZDIM = 256;
	pVolume = NULL; 

} 

VolumeSplatter::~VolumeSplatter(void)
{ 
	if(pVolume!=NULL) {
		delete [] pVolume;
		pVolume = NULL;
	}
}

void VolumeSplatter::SetVolumeDimensions(const int xdim, const int ydim, const int zdim) {
	XDIM = xdim;
	YDIM = ydim;
	ZDIM = zdim;
	invDim.x = 1.0f/XDIM; 
	invDim.y = 1.0f/YDIM; 
	invDim.z = 1.0f/ZDIM; 
}
void VolumeSplatter::SetNumSamplingVoxels(const int x, const int y, const int z) {
	X_SAMPLING_DIST = x;
	Y_SAMPLING_DIST = y;
	Z_SAMPLING_DIST = z;
}
void VolumeSplatter::SetIsosurfaceValue(const GLubyte value) {
	isoValue = value;
}

bool VolumeSplatter::LoadVolume(const std::string& filename) {
	std::ifstream infile(filename.c_str(), std::ios_base::binary); 

	if(infile.good()) {
		pVolume = new GLubyte[XDIM*YDIM*ZDIM];
		infile.read(reinterpret_cast<char*>(pVolume), XDIM*YDIM*ZDIM*sizeof(GLubyte));
		infile.close();
		return true;
	} else {
		return false;
	}
} 

void VolumeSplatter::SampleVoxel(const int x, const int y, const int z) {
	GLubyte data = SampleVolume(x, y, z);
	if(data>isoValue) {
		Vertex v; 
		v.pos.x = (float)x;
		v.pos.y = (float)y;
		v.pos.z = (float)z;			 			
		v.normal = GetNormal(x, y, z);
		v.pos *= invDim; 
		vertices.push_back(v);
	} 
}

void VolumeSplatter::SplatVolume() {
	vertices.clear(); 
	int dx = XDIM/X_SAMPLING_DIST;
	int dy = YDIM/Y_SAMPLING_DIST;
	int dz = ZDIM/Z_SAMPLING_DIST;
	scale = glm::vec3(dx,dy,dz); 
	for(int z=0;z<ZDIM;z+=dz) {
		for(int y=0;y<YDIM;y+=dy) {
			for(int x=0;x<XDIM;x+=dx) {
				SampleVoxel(x,y,z);
			}
		}
	}
}
  
size_t VolumeSplatter::GetTotalVertices() {
	return vertices.size();
}
Vertex* VolumeSplatter::GetVertexPointer() {
	return  &vertices[0];
} 

GLubyte VolumeSplatter::SampleVolume(const int x, const int y, const int z) {
  	int index = (x+(y*XDIM)) + z*(XDIM*YDIM); 
	if(index<0)
		index = 0;
	if(index >= XDIM*YDIM*ZDIM)
		index = (XDIM*YDIM*ZDIM)-1;
	return pVolume[index];
}

glm::vec3 VolumeSplatter::GetNormal (const int x, const int y, const int z) { 
	glm::vec3 N;
	N.x =  (SampleVolume(int(x-scale.x),y,z)-SampleVolume(int(x+scale.x),y,z))/(2*scale.x)  ;
	N.y =  (SampleVolume(x,int(y-scale.y),z)-SampleVolume(x,int(y+scale.y),z))/(2*scale.y) ;
	N.z =  (SampleVolume(x,y,int(z-scale.z))-SampleVolume(x,y,int(z+scale.z)))/(2*scale.z) ;
	return glm::normalize(N);
} 