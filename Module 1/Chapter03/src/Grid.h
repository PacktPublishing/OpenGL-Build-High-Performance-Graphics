#pragma once
#include "RenderableObject.h"

class CGrid:public RenderableObject
{
public:
	CGrid(int width=10, int depth=10);
	virtual ~CGrid(void); 
	 
	int GetTotalVertices();
	int GetTotalIndices(); 
	GLenum GetPrimitiveType();

	void FillVertexBuffer( GLfloat* pBuffer);
	void FillIndexBuffer( GLuint* pBuffer);

private:
	int width, depth;
};

