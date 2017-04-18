#include "Quad.h"
#include <glm/glm.hpp>
 
CQuad::CQuad(float zpos)
{
	//generate the cube object
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/quad_shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/quad_shader.frag");
	shader.CreateAndLinkProgram();
	shader.Use();
		shader.AddAttribute("vVertex"); 
		shader.AddAttribute("vUV");  
		shader.AddUniform("MVP"); 
		shader.AddUniform("textureMap"); 
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();
	Zpos = zpos;

	/*
	glm::vec3* vertices=new glm::vec3[4]; 
	vertices[0] = glm::vec3( -1,0, zpos);
	vertices[1] = glm::vec3( 1, 0, zpos);
	vertices[2] = glm::vec3( 1, 2, zpos);
	vertices[3] = glm::vec3( -1, 2, zpos); 
		 
	glm::vec3 center = vertices[0];
	center += vertices[1];
	center += vertices[2];
	center += vertices[3];
	position.x = center.x/4.0f;
	position.y = center.y/4.0f;
	position.z = center.z/4.0f;

	glm::vec3 e1 = vertices[1]-vertices[0];
	glm::vec3 e2 = vertices[2]-vertices[0];
	normal  = glm::cross(e1,e2); 

	total_indices = 2*3;
	GLushort* indices = new GLushort[total_indices];
	 
	int count = 0;
	
	//fill indices array
	GLushort* id=&indices[0]; 
	 
	*id++ = 0; 	*id++ = 1; 	*id++ = 2;
	*id++ = 0; 	*id++ = 2; 	*id++ = 3; 
	 
	//setup vao and vbo stuff
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);	

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		glBufferData (GL_ARRAY_BUFFER, 8*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		 
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		  
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*total_indices, &indices[0], GL_STATIC_DRAW);
		 
	glBindVertexArray(0);
	delete [] indices;
	delete [] vertices;
 */
	Init();
}


CQuad::~CQuad(void)
{
}
/*
void CQuad::Render(const float* MVP) {
	shader.Use();				 
	glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, MVP );
	
	glBindVertexArray(vaoID);
		glDrawElements(GL_TRIANGLES, total_indices, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	shader.UnUse();
} */
int CQuad::GetTotalVertices() {
	return 4;
}

int CQuad::GetTotalIndices() {
	return 6;
}

GLenum CQuad::GetPrimitiveType() {
	return GL_TRIANGLES;
}

void CQuad::FillVertexBuffer(GLfloat* pBuffer) {
	glm::vec3* vertices = (glm::vec3*)(pBuffer); 
	vertices[0] = glm::vec3( -1,0, Zpos);
	vertices[1] = glm::vec3( 1, 0, Zpos);
	vertices[2] = glm::vec3( 1, 2, Zpos);
	vertices[3] = glm::vec3( -1, 2, Zpos); 
		 
	glm::vec3 center = vertices[0];
	center += vertices[1];
	center += vertices[2];
	center += vertices[3];
	position.x = center.x/4.0f;
	position.y = center.y/4.0f;
	position.z = center.z/4.0f;

	glm::vec3 e1 = vertices[1]-vertices[0];
	glm::vec3 e2 = vertices[2]-vertices[0];
	normal  = glm::cross(e1,e2); 
}

void CQuad::FillIndexBuffer(GLuint* pBuffer) {
	//fill indices array
	GLuint* id=pBuffer; 
	*id++ = 0; 
	*id++ = 1; 
	*id++ = 2;
	*id++ = 0;  
	*id++ = 2;  
	*id++ = 3;  
}