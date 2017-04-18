#include "UnitCube.h"

#include <glm/gtc/type_ptr.hpp>

CUnitCube::CUnitCube(const glm::vec3& col)
{
	color = col;
	//generate the cube object
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/cube_shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/cube_shader.frag");
	shader.CreateAndLinkProgram();
	shader.Use();
		shader.AddAttribute("vVertex"); 
		shader.AddUniform("MVP");
		shader.AddUniform("vColor");
		glUniform3fv(shader("vColor"),1, glm::value_ptr(color));
	shader.UnUse();
	 
	Init();
}

void CUnitCube::SetCustomUniforms() {
	glUniform3fv(shader("vColor"),1, glm::value_ptr(color));
}

CUnitCube::~CUnitCube(void)
{
	 
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
	glm::vec3* vertices = (glm::vec3*)(pBuffer); 
	vertices[0]=glm::vec3(-0.5f,-0.5f,-0.5f);
	vertices[1]=glm::vec3( 0.5f,-0.5f,-0.5f);
	vertices[2]=glm::vec3( 0.5f, 0.5f,-0.5f);
	vertices[3]=glm::vec3(-0.5f, 0.5f,-0.5f);
	vertices[4]=glm::vec3(-0.5f,-0.5f, 0.5f);
	vertices[5]=glm::vec3( 0.5f,-0.5f, 0.5f);
	vertices[6]=glm::vec3( 0.5f, 0.5f, 0.5f);
	vertices[7]=glm::vec3(-0.5f, 0.5f, 0.5f); 
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