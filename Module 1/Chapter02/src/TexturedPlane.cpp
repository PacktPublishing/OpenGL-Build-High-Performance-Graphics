#include "TexturedPlane.h"
#include <glm/glm.hpp>

CTexturedPlane::CTexturedPlane(const int w, const int d)
{
	width = w;
	depth = d;

	//setup shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/checker_shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/checker_shader.frag");
	shader.CreateAndLinkProgram();
	shader.Use();	
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP"); 
		shader.AddUniform("textureMap"); 
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();
 
	Init();
}


CTexturedPlane::~CTexturedPlane(void)
{
}

int CTexturedPlane::GetTotalVertices() {
	return 4;
}

int CTexturedPlane::GetTotalIndices() {
	return 6;
}

GLenum CTexturedPlane::GetPrimitiveType() {
	return GL_TRIANGLES;
}

void CTexturedPlane::FillVertexBuffer(GLfloat* pBuffer) {
	glm::vec3* vertices = (glm::vec3*)(pBuffer);
	
	int width_2 = width/2;
	int depth_2 = depth/2;
	 
	vertices[0] = glm::vec3( -width_2, 0,-depth_2);
	vertices[1] = glm::vec3( width_2,0, -depth_2);

	vertices[2] = glm::vec3( width_2,0,depth_2);
	vertices[3] = glm::vec3( -width_2,0,depth_2); 
}

void CTexturedPlane::FillIndexBuffer(GLuint* pBuffer) {
	
	//fill indices array
	GLuint* id=pBuffer; 
	*id++ = 0; 
	*id++ = 1; 
	*id++ = 2;
	*id++ = 0;
	*id++ = 2;
	*id++ = 3;
}
 