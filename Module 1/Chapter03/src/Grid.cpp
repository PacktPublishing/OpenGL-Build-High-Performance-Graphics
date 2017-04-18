#include "Grid.h"
#include <glm/glm.hpp>

CGrid::CGrid(int width, int depth)
{
	this->width = width;
	this->depth = depth;

	//setup shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	shader.CreateAndLinkProgram();
	shader.Use();	
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP"); 
	shader.UnUse();

	/*
	glm::vec3* vertices=new glm::vec3[((width+1)+(depth+1))*2];
	total_indices = width*depth;
	GLushort* indices = new GLushort[total_indices];
	 
	int count = 0;
	int width_2 = width/2;
	int depth_2 = depth/2;
	int i=0 ;

	for( i=-width_2;i<=width_2;i++) {		  
		vertices[count++] = glm::vec3( i,0,-depth_2);
		vertices[count++] = glm::vec3( i,0, depth_2);

		vertices[count++] = glm::vec3( -width_2,0,i);
		vertices[count++] = glm::vec3(  width_2,0,i);
	}

	//fill indices array
	GLushort* id=&indices[0]; 
	for (i = 0; i < width*depth; i+=4) {            
		*id++ = i; 
		*id++ = i+1; 
		*id++ = i+2;
		*id++ = i+3; 
	}

	//setup vao and vbo stuff
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);
	 
	glBindVertexArray(vaoID);	

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		glBufferData (GL_ARRAY_BUFFER, ((width+1)+(depth+1))*2*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		 
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


CGrid::~CGrid(void)
{
	
} 

int CGrid::GetTotalVertices() {
	return ((width+1)+(depth+1))*2;
}

int CGrid::GetTotalIndices() {
	return (width*depth);
}

GLenum CGrid::GetPrimitiveType() {
	return GL_LINES;
}

void CGrid::FillVertexBuffer(GLfloat* pBuffer) {
	glm::vec3* vertices = (glm::vec3*)(pBuffer);
	int count = 0;
	int width_2 = width/2;
	int depth_2 = depth/2;
	int i=0 ;

	for( i=-width_2;i<=width_2;i++) {		  
		vertices[count++] = glm::vec3( i,0,-depth_2);
		vertices[count++] = glm::vec3( i,0, depth_2);

		vertices[count++] = glm::vec3( -width_2,0,i);
		vertices[count++] = glm::vec3(  width_2,0,i);
	}
}

void CGrid::FillIndexBuffer(GLuint* pBuffer) {
	int i=0;
	//fill indices array
	GLuint* id=pBuffer; 
	for (i = 0; i < width*depth; i+=4) {            
		*id++ = i; 
		*id++ = i+1; 
		*id++ = i+2;
		*id++ = i+3; 
	}
}