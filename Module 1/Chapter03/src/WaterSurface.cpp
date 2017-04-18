#define _USE_MATH_DEFINES
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "WaterSurface.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


float random(float min, float max) {
    
    double n = (double) rand() / (double) RAND_MAX;
    double v = min + n * (max - min);
    return v;
}

CWaterSurface::CWaterSurface(int w, int d, float x, float z)
{
	srand(::time(NULL));

	width = w;
	depth = d;

	wsSizeX = x;
	wsSizeZ = z;

	glm::vec2 directions[4];
	for(int i=0;i<4;i++) {
		float angle = random(-M_PI/3.0f, M_PI/3.0f);
		directions[i]=glm::vec2(cos(angle),sin(angle));
	}

	//generate the cube object
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/water.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/water.frag");
	shader.CreateAndLinkProgram();
	shader.Use();
		shader.AddAttribute("vVertex");  
		shader.AddUniform("MVP"); 
		shader.AddUniform("time");
		shader.AddUniform("eyePos");
		shader.AddUniform("directions");
		glUniform2fv(shader("directions"),4,glm::value_ptr(directions[0]));
	shader.UnUse();  
	Init();
}


void CWaterSurface::SetTime(const float t) {
	time = t;
} 

void CWaterSurface::SetEyePos(const glm::vec3& ePos) {
	eyePos = ePos;
}

void CWaterSurface::SetCustomUniforms() {
	glUniform1f(shader("time"), time);   
}

CWaterSurface::~CWaterSurface(void)
{
} 

int CWaterSurface::GetTotalVertices() {
	return ((width+1)*(depth+1));
}

int CWaterSurface::GetTotalIndices() {
	return width*depth*2*3;
}

GLenum CWaterSurface::GetPrimitiveType() {
	return GL_TRIANGLES;
}

void CWaterSurface::FillVertexBuffer(GLfloat* pBuffer) {
	glm::vec3* vertices = (glm::vec3*)(pBuffer);
	int count = 0;
	float width_2 = wsSizeX/2.0f;
	float depth_2 = wsSizeZ/2.0f;
	int i=0, j=0;
	for( j=0;j<=depth;j++) {		 
		for( i=0;i<=width;i++) {	 
			vertices[count++] = glm::vec3( ((float(i)/(width-1)) *2-1)* width_2, 0, ((float(j)/(depth-1))*2-1)*depth_2);
		}
	}
}

void CWaterSurface::FillIndexBuffer(GLuint* pBuffer) { 
	int i=0, j=0;
	//fill indices array
	GLuint* id=pBuffer; 
	for (i = 0; i < depth; i++) {        
		for (j = 0; j < width; j++) {            
			int i0 = i * (width+1) + j;            
			int i1 = i0 + 1;            
			int i2 = i0 + (width+1);            
			int i3 = i2 + 1;            
			if ((j+i)%2) {                
				*id++ = i0; *id++ = i2; *id++ = i1;                
				*id++ = i1; *id++ = i2; *id++ = i3;            
			} else {                
				*id++ = i0; *id++ = i2; *id++ = i3;                
				*id++ = i0; *id++ = i3; *id++ = i1;            
			}        
		}    
	}
}
 