#pragma once
#include "RenderableObject.h"
 
class CUnitColorCube:public RenderableObject
{
public:
	CUnitColorCube(void);
	virtual ~CUnitColorCube(void);

	int GetTotalVertices();
	int GetTotalIndices(); 
	GLenum GetPrimitiveType();

	void FillVertexBuffer( GLfloat* pBuffer);
	void FillIndexBuffer( GLuint* pBuffer); 
};

