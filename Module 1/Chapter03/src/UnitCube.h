#pragma once
#include "RenderableObject.h"
 #include <glm/glm.hpp>
class CUnitCube:public RenderableObject
{
public: 
	CUnitCube(const glm::vec3& color=glm::vec3(1,1,1));
	virtual ~CUnitCube(void);

	int GetTotalVertices();
	int GetTotalIndices(); 
	GLenum GetPrimitiveType();
	void SetCustomUniforms();

	void FillVertexBuffer( GLfloat* pBuffer);
	void FillIndexBuffer( GLuint* pBuffer); 

	glm::vec3 color;
};

