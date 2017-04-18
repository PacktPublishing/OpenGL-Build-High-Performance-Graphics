#pragma once
#include "renderableobject.h"
#include <glm/glm.hpp>

class CQuad :
	public RenderableObject
{
public:
	CQuad(float zpos=0);
	virtual ~CQuad(void);

	int GetTotalVertices();
	int GetTotalIndices(); 
	GLenum GetPrimitiveType();

	void FillVertexBuffer( GLfloat* pBuffer);
	void FillIndexBuffer( GLuint* pBuffer);

	float Zpos;
	glm::vec3 position;
	glm::vec3 normal;
};

