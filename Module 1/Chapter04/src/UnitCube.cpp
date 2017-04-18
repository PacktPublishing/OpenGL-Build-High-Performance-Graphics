#include "UnitCube.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

CUnitCube::CUnitCube(const glm::vec3& col)
{
	color = col;
	//generate the cube object
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/per_vertex_light.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/per_vertex_light.frag");
	shader.CreateAndLinkProgram();
	shader.Use();
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MVP");
		shader.AddUniform("MV");
		shader.AddUniform("N");
		shader.AddUniform("vColor");
		glUniform3fv(shader("vColor"),1, glm::value_ptr(color));
	shader.UnUse();
	 
	Init();
}

void CUnitCube::SetCustomUniforms() {
	glUniform3fv(shader("vColor"),1, glm::value_ptr(color));
	glUniformMatrix4fv(shader("MV"),1,GL_FALSE, glm::value_ptr(MV));
	glUniformMatrix3fv(shader("N"),1,GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
}

CUnitCube::~CUnitCube(void)
{
	 
} 

void CUnitCube::SetModelViewMatrix(const glm::mat4& gMV) {
	MV = gMV;
}
int CUnitCube::GetTotalVertices() {
	return 8;
}

int CUnitCube::GetTotalIndices() {
	//6 faces with 2 triangles each with 3 vertices
	return 6*2*3;
}

GLenum CUnitCube::GetPrimitiveType() {
	return GL_TRIANGLES;
}

 void CUnitCube::FillVertexBuffer(GLfloat* pBuffer) {
	Vertex* vertices = (Vertex*)(pBuffer); 
	vertices[0].pos = glm::vec3(-0.5f,-0.5f,-0.5f);
	vertices[1].pos = glm::vec3( 0.5f,-0.5f,-0.5f);
	vertices[2].pos = glm::vec3( 0.5f, 0.5f,-0.5f);
	vertices[3].pos = glm::vec3(-0.5f, 0.5f,-0.5f);
	vertices[4].pos = glm::vec3(-0.5f,-0.5f, 0.5f);
	vertices[5].pos = glm::vec3( 0.5f,-0.5f, 0.5f);
	vertices[6].pos = glm::vec3( 0.5f, 0.5f, 0.5f);
	vertices[7].pos = glm::vec3(-0.5f, 0.5f, 0.5f); 
}

void CUnitCube::FillIndexBuffer(GLuint* pBuffer) {
	 
	//fill indices array
	GLuint* id=pBuffer; 

	//bottom face
	*id++ = 0; 	*id++ = 5; 	*id++ = 4;
	*id++ = 5; 	*id++ = 0; 	*id++ = 1; 
	
	//top face
	*id++ = 3; 	*id++ = 7; 	*id++ = 6;
	*id++ = 3; 	*id++ = 6; 	*id++ = 2;

	//front face
	*id++ = 7; 	*id++ = 4; 	*id++ = 6;
	*id++ = 6; 	*id++ = 4; 	*id++ = 5;

	//back face
	*id++ = 2; 	*id++ = 1; 	*id++ = 3;
	*id++ = 3; 	*id++ = 1; 	*id++ = 0;

	//left face 
	*id++ = 3; 	*id++ = 0; 	*id++ = 7;
	*id++ = 7; 	*id++ = 0; 	*id++ = 4;

	//right face 
	*id++ = 6; 	*id++ = 5; 	*id++ = 2;
	*id++ = 2; 	*id++ = 5; 	*id++ = 1;
}