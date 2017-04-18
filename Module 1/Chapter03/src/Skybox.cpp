#include "Skybox.h"

#include <glm/gtc/type_ptr.hpp>

CSkybox::CSkybox(void)
{ 
	//generate the cube object
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/skybox.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/skybox.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add shader attributes and uniforms
		shader.AddAttribute("vVertex"); 
		shader.AddUniform("MVP");
		shader.AddUniform("cubeMap");
		//set constant shader uniforms at initialization
		glUniform1i(shader("cubeMap"),0);
	shader.UnUse();
	 
	//setup the parent's fields
	Init();
}
 

CSkybox::~CSkybox(void)
{
	 
} 

//there are 8 vertices in a skybox
int CSkybox::GetTotalVertices() {
	return 8;
}

int CSkybox::GetTotalIndices() {
	//6 faces with 2 triangles each with 3 vertices
	return 6*2*3;
}

GLenum CSkybox::GetPrimitiveType() {
	return GL_TRIANGLES;
}

 void CSkybox::FillVertexBuffer(GLfloat* pBuffer) {
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

void CSkybox::FillIndexBuffer(GLuint* pBuffer) {
	 
	//fill indices array
	GLuint* id=pBuffer; 

	//bottom face
	*id++ = 0; 	*id++ = 4; 	*id++ = 5;
	*id++ = 5; 	*id++ = 1; 	*id++ = 0; 
	
	//top face
	*id++ = 3; 	*id++ = 6; 	*id++ = 7;
	*id++ = 3; 	*id++ = 2; 	*id++ = 6;

	//front face
	*id++ = 7; 	*id++ = 6; 	*id++ = 4;
	*id++ = 6; 	*id++ = 5; 	*id++ = 4;

	//back face
	*id++ = 2; 	*id++ = 3; 	*id++ = 1;
	*id++ = 3; 	*id++ = 0; 	*id++ = 1;

	//left face 
	*id++ = 3; 	*id++ = 7; 	*id++ = 0;
	*id++ = 7; 	*id++ = 4; 	*id++ = 0;

	//right face 
	*id++ = 6; 	*id++ = 2; 	*id++ = 5;
	*id++ = 2; 	*id++ = 1; 	*id++ = 5;
}