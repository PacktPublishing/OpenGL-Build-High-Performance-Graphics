#pragma once
#include "RenderableObject.h"
 #include <glm/glm.hpp>

class CSkybox:public RenderableObject
{
public: 
	CSkybox(void);
	virtual ~CSkybox(void);

	int GetTotalVertices();
	int GetTotalIndices(); 
	GLenum GetPrimitiveType(); 
	void FillVertexBuffer( GLfloat* pBuffer);
	void FillIndexBuffer( GLuint* pBuffer);  
	 
};

