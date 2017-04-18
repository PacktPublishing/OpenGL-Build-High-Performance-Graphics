#pragma once
#include "renderableobject.h"
#include <glm/glm.hpp>

class CWaterSurface:
	public RenderableObject
{
public:
	CWaterSurface(int width=100, int depth=100, float wsW=4, float wsH=4);
	virtual ~CWaterSurface(void);

	int GetTotalVertices();
	int GetTotalIndices(); 
	GLenum GetPrimitiveType();

	void FillVertexBuffer( GLfloat* pBuffer);
	void FillIndexBuffer( GLuint* pBuffer); 

	void SetCustomUniforms();

	void SetTime(const float t);  
	void SetEyePos(const glm::vec3& eyePos);

private:
	int width, depth;
	float wsSizeX, wsSizeZ;
	float time; 
	glm::vec3 eyePos;
};

