#pragma once
#include "GLSLShader.h"
 

class RenderableObject
{
public:
	RenderableObject(void);
	virtual ~RenderableObject(void);
	void Render(const float* MVP);
	
	virtual int GetTotalVertices()=0;
	virtual int GetTotalIndices()=0;
	virtual GLenum GetPrimitiveType() =0; 

	virtual void FillVertexBuffer(GLfloat* pBuffer)=0;
	virtual void FillIndexBuffer(GLuint* pBuffer)=0;
	
	void Init();
	void Destroy();

	virtual void SetCustomUniforms();
	GLSLShader* GetShader();

protected:
	GLuint vaoID;
	GLuint vboVerticesID;
	GLuint vboIndicesID;
	
	GLSLShader shader;

	GLenum primType;
	int totalVertices, totalIndices;
};

