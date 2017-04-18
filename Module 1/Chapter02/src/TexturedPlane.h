#pragma once
#include "d:\mybook\codes\chapter2\src\renderableobject.h"
class CTexturedPlane :
	public RenderableObject
{
public:
	CTexturedPlane(const int width=1000, const int depth=1000);
	virtual ~CTexturedPlane(void);
	int GetTotalVertices();
	int GetTotalIndices(); 
	GLenum GetPrimitiveType();

	void FillVertexBuffer( GLfloat* pBuffer);
	void FillIndexBuffer( GLuint* pBuffer);
	 
private:
	int width, depth;
};

