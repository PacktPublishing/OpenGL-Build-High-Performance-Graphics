#include <GL/glew.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "../src/GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//screen dimensions
const int WIDTH  = 1024;
const int HEIGHT = 768;

//shadowmap texture dimensions
const int SHADOWMAP_WIDTH = 512;
const int SHADOWMAP_HEIGHT = 512;

//shadowmapping and flat shader
GLSLShader shader;
GLSLShader flatshader;

//vertex struct with position and normal
struct Vertex {
	glm::vec3 pos, normal;
};

//sphere vertex array and vertex buffer object IDs
GLuint sphereVAOID;
GLuint sphereVerticesVBO;
GLuint sphereIndicesVBO;

//cube vertex array and vertex buffer object IDs
GLuint cubeVAOID;
GLuint cubeVerticesVBO;
GLuint cubeIndicesVBO;

//plane vertex array and vertex buffer object IDs
GLuint planeVAOID;
GLuint planeVerticesVBO;
GLuint planeIndicesVBO;

//light crosshair gizmo vertex array and vertex buffer objects
GLuint lightVAOID;
GLuint lightVerticesVBO;

//projection, modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4  MV = glm::mat4(1);

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -10;

glm::vec3 lightPosOS=glm::vec3(0,2,0); //objectspace light position

#include <vector>

//vertices and indices for sphere/cube
std::vector<Vertex> vertices;
std::vector<GLushort> indices;
int totalSphereTriangles = 0;

//spherical coordinates for storing the light position
float theta = -7;
float phi = -0.77f;
float radius = 5;

//shadow map texture ID
GLuint shadowMapTexID;

//FBO ID
GLuint fboID;

glm::mat4 MV_L; //light modelview matrix
glm::mat4 P_L;	//light projection matrix
glm::mat4 B;    //light bias matrix
glm::mat4 BP;   //light bias and projection matrix combined
glm::mat4 S;    //light's combined MVPB matrix

//adds the given sphere indices to the indices vector
inline void push_indices(int sectors, int r, int s, std::vector<GLushort>& indices) {
    int curRow = r * sectors;
    int nextRow = (r+1) * sectors;

    indices.push_back(curRow + s);
    indices.push_back(nextRow + s);
    indices.push_back(nextRow + (s+1));

    indices.push_back(curRow + s);
    indices.push_back(nextRow + (s+1));
    indices.push_back(curRow + (s+1));
}

//generates a sphere primitive with the given radius, slices and stacks
void CreateSphere( float radius, unsigned int slices, unsigned int stacks, std::vector<Vertex>& vertices, std::vector<GLushort>& indices)
{
    float const R = 1.0f/(float)(slices-1);
    float const S = 1.0f/(float)(stacks-1);

    for(size_t r = 0; r < slices; ++r) {
        for(size_t s = 0; s < stacks; ++s) {
            float const y = (float)(sin( -M_PI_2 + M_PI * r * R ));
            float const x = (float)(cos(2*M_PI * s * S) * sin( M_PI * r * R ));
            float const z = (float)(sin(2*M_PI * s * S) * sin( M_PI * r * R ));

			Vertex v;
			v.pos = glm::vec3(x,y,z)*radius;
			v.normal = glm::normalize(v.pos);
            vertices.push_back(v);
            push_indices(stacks, r, s, indices);
        }
    }

}

//generates a cube of the given size
void CreateCube(const float& size, std::vector<Vertex>& vertices, std::vector<GLushort>& indices) {
	float halfSize = size/2.0f;
	glm::vec3 positions[8];
	positions[0]=glm::vec3(-halfSize,-halfSize,-halfSize);
	positions[1]=glm::vec3( halfSize,-halfSize,-halfSize);
	positions[2]=glm::vec3( halfSize, halfSize,-halfSize);
	positions[3]=glm::vec3(-halfSize, halfSize,-halfSize);
	positions[4]=glm::vec3(-halfSize,-halfSize, halfSize);
	positions[5]=glm::vec3( halfSize,-halfSize, halfSize);
	positions[6]=glm::vec3( halfSize, halfSize, halfSize);
	positions[7]=glm::vec3(-halfSize, halfSize, halfSize);

	glm::vec3 normals[6];
	normals[0]=glm::vec3(-1.0,0.0,0.0);
	normals[1]=glm::vec3(1.0,0.0,0.0);
	normals[2]=glm::vec3(0.0,1.0,0.0);
	normals[3]=glm::vec3(0.0,-1.0,0.0);
	normals[4]=glm::vec3(0.0,0.0,1.0);
	normals[5]=glm::vec3(0.0,0.0,-1.0);

	indices.resize(36);
	vertices.resize(36);

	//fill indices array
	GLushort* id=&indices[0];
	//left face
	*id++ = 7; 	*id++ = 3; 	*id++ = 4;
	*id++ = 3; 	*id++ = 0; 	*id++ = 4;

	//right face
	*id++ = 2; 	*id++ = 6; 	*id++ = 1;
	*id++ = 6; 	*id++ = 5; 	*id++ = 1;

	//top face
	*id++ = 7; 	*id++ = 6; 	*id++ = 3;
	*id++ = 6; 	*id++ = 2; 	*id++ = 3;
	//bottom face
	*id++ = 0; 	*id++ = 1; 	*id++ = 4;
	*id++ = 1; 	*id++ = 5; 	*id++ = 4;

	//front face
	*id++ = 6; 	*id++ = 4; 	*id++ = 5;
	*id++ = 6; 	*id++ = 7; 	*id++ = 4;
	//back face
	*id++ = 0; 	*id++ = 2; 	*id++ = 1;
	*id++ = 0; 	*id++ = 3; 	*id++ = 2;


	for(int i=0;i<36;i++) {
		int normal_index = i/6;
		vertices[i].pos=positions[indices[i]];
		vertices[i].normal=normals[normal_index];
		indices[i]=i;
	}
}

//generates a plane of the given width and depth
void CreatePlane(const float width, const float depth, std::vector<Vertex>& vertices, std::vector<GLushort>& indices) {
	float halfW = width/2.0f;
	float halfD = depth/2.0f;

	indices.resize(6);
	vertices.resize(4);
	glm::vec3 normal=glm::vec3(0,1,0);

	vertices[0].pos = glm::vec3(-halfW,0.01,-halfD); vertices[0].normal=normal;
	vertices[1].pos = glm::vec3(-halfW,0.01, halfD); vertices[1].normal=normal;
	vertices[2].pos = glm::vec3( halfW,0.01, halfD); vertices[2].normal=normal;
	vertices[3].pos = glm::vec3( halfW,0.01,-halfD); vertices[3].normal=normal;

	//fill indices array
	indices[0]=0;
	indices[1]=1;
	indices[2]=2;

	indices[3]=0;
	indices[4]=2;
	indices[5]=3;
}

//mouse click handler
void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1;
}

//mouse move handler
void OnMouseMove(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f);
	else if(state ==2) {
		theta += (oldX - x)/60.0f;
		phi += (y - oldY)/60.0f;

		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

		//update light's MV matrix
		MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
		S = BP*MV_L;
	} else {
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//OpenGL initialization
void OnInit() {
	
	//load the flat shader
	flatshader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	flatshader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
	flatshader.CreateAndLinkProgram();
	flatshader.Use();
		//add attributes and uniforms
		flatshader.AddAttribute("vVertex");
		flatshader.AddUniform("MVP");
	flatshader.UnUse();

	//load the shadow mapping PCF shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/PointLightShadowMapped_PCF.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/PointLightShadowMapped_PCF.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MVP");
		shader.AddUniform("MV");
		shader.AddUniform("N");
		shader.AddUniform("S");
		shader.AddUniform("light_position");
		shader.AddUniform("diffuse_color");
		shader.AddUniform("bIsLightPass");
		shader.AddUniform("shadowMap");
		//pass value of constant uniforms at initialization
		glUniform1i(shader("shadowMap"),0);
	shader.UnUse();

	GL_CHECK_ERRORS

	//setup sphere geometry
	CreateSphere(1.0f,10,10, vertices, indices);

	//setup sphere vao and vbo stuff
	glGenVertexArrays(1, &sphereVAOID);
	glGenBuffers(1, &sphereVerticesVBO);
	glGenBuffers(1, &sphereIndicesVBO);
	glBindVertexArray(sphereVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, sphereVerticesVBO);
		//pass vertices to the buffer object
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//enable vertex attribute array for normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS
		//pass sphere indices to element array buffer 
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	//store the total number of sphere triangles
	totalSphereTriangles = indices.size();

	//clear the vertices and indices vectors as we will reuse them
	//for cubes
	vertices.clear();
	indices.clear();

	//setup cube geometry
	CreateCube(2,vertices, indices);

	//setup cube vao and vbo stuff
	glGenVertexArrays(1, &cubeVAOID);
	glGenBuffers(1, &cubeVerticesVBO);
	glGenBuffers(1, &cubeIndicesVBO);
	glBindVertexArray(cubeVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, cubeVerticesVBO);
		//pass vertices to the buffer object
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//enable vertex attribute array for normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS
		//pass cube indices to element array buffer
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cubeIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//clear the vertices and indices vectors as we will reuse them for plane
	vertices.clear();
	indices.clear();
	//create a plane object
	CreatePlane(100,100,vertices, indices);

	//setup plane VAO and VBO
	glGenVertexArrays(1, &planeVAOID);
	glGenBuffers(1, &planeVerticesVBO);
	glGenBuffers(1, &planeIndicesVBO);
	glBindVertexArray(planeVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, planeVerticesVBO);
		//pass vertices to the buffer object
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//enable vertex attribute array for normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS
		//pass plane indices to element array buffer
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, planeIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//setup vao and vbo stuff for the light position crosshair
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//setup light gizmo vertex array and buffer object
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
		//pass light crosshair gizmo vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);


	GL_CHECK_ERRORS

	//get light position from spherical coordinates
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//setup the shadowmap texture
	glGenTextures(1, &shadowMapTexID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexID);

	//set texture parameters
	GLfloat border[4]={1,0,0,0};
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,GL_COMPARE_REF_TO_TEXTURE);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL);
	glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,SHADOWMAP_WIDTH,SHADOWMAP_HEIGHT,0,GL_DEPTH_COMPONENT,GL_UNSIGNED_BYTE,NULL);

	//set up FBO to get the depth component
	glGenFramebuffers(1,&fboID);
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,shadowMapTexID,0);

	//check framebuffer completeness status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"FBO setup successful."<<endl;
	} else {
		cout<<"Problem in FBO setup."<<endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	//set the light MV, P and bias matrices
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	P_L  = glm::perspective(50.0f,1.0f,1.0f, 25.0f);
	B    = glm::scale(glm::translate(glm::mat4(1),glm::vec3(0.5,0.5,0.5)), glm::vec3(0.5,0.5,0.5));
	BP   = B*P_L;
	S    = BP*MV_L;
	
	//enable depth testing and culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {

	glDeleteTextures(1, &shadowMapTexID);
	//Destroy shader
	shader.DeleteShaderProgram();
	flatshader.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1, &sphereVerticesVBO);
	glDeleteBuffers(1, &sphereIndicesVBO);
	glDeleteVertexArrays(1, &sphereVAOID);

	glDeleteBuffers(1, &cubeVerticesVBO);
	glDeleteBuffers(1, &cubeIndicesVBO);
	glDeleteVertexArrays(1, &cubeVAOID);

	glDeleteBuffers(1, &planeVerticesVBO);
	glDeleteBuffers(1, &planeIndicesVBO);
	glDeleteVertexArrays(1, &planeVAOID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);

	glDeleteFramebuffers(1,&fboID);

	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.1f, 1000.f);
}

//idle callback just calls the display function
void OnIdle() {
	glutPostRedisplay();
}

//scene rendering function
void DrawScene(glm::mat4 View, glm::mat4 Proj, int isLightPass = 1) {

	GL_CHECK_ERRORS

	//bind the current shader
	shader.Use();
		//render plane
		glBindVertexArray(planeVAOID); {
			//set the shader uniforms
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniform1i(shader("bIsLightPass"), isLightPass);
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(Proj*View));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(View));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(View))));
			glUniform3f(shader("diffuse_color"), 1.0f,1.0f,1.0f);
				//draw plane triangles
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

		//draw the cube
		glBindVertexArray(cubeVAOID); {
			//set the cube's transform
			glm::mat4 T = glm::translate(glm::mat4(1),  glm::vec3(-1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
			//pass shader uniforms
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(M));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniform3f(shader("diffuse_color"), 1.0f,0.0f,0.0f);
				//draw cube triangles
	 			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		}
		//render the sphere
		glBindVertexArray(sphereVAOID); {
			//set the sphere's transform
			glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
			//set the shader uniforms
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(M));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniform3f(shader("diffuse_color"), 0.0f, 0.0f, 1.0f);
				//draw sphere triangles
		 		glDrawElements(GL_TRIANGLES, totalSphereTriangles, GL_UNSIGNED_SHORT, 0);
		}
	
	//unbind shader
	shader.UnUse();

	GL_CHECK_ERRORS
}

//display callback function
void OnRender() {

	GL_CHECK_ERRORS

	//clear colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transform
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	 
	//1) Render scene from the point of view of light
	//enable rendering to FBO
 	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	//clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);
	//reset viewport to the shadow map texture size
	glViewport(0,0,SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	//enable front face culling
	glCullFace(GL_FRONT);
		DrawScene(MV_L, P_L);
	glCullFace(GL_BACK);

	//restore normal rendering path
	//unbind FBO, set the default back buffer and reset the viewport to screen size
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glDrawBuffer(GL_BACK_LEFT);
	glViewport(0,0,WIDTH, HEIGHT);
	
	//2) Render scene from point of view of eye
	glViewport(0,0,WIDTH, HEIGHT);
	    DrawScene(MV, P, 0 );

 	//bind light gizmo vertex array object
	glBindVertexArray(lightVAOID); {
		//set the flat shader
		flatshader.Use();
			//set the light's transform and render 3 lines
			glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
			glUniformMatrix4fv(flatshader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				glDrawArrays(GL_LINES, 0, 6);
		//unbind shader
		flatshader.UnUse();
	}
	
	//unbind the vertex array object
	glBindVertexArray(0);	

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}


//mouse wheel scroll handler which changes the radius of the light source
//since the position is given in spherical coordinates, the radius contols 
//how far from the center the light source is.
void OnMouseWheel(int button, int dir, int x, int y) {

	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }

	radius = max(radius,0.0f);

	//get the new light position
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//update light's MV matrix
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	S = BP*MV_L;

	//call the display function
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Shadow Mapping (PCF) - OpenGL 3.3");
	
	//glew initialization
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
		cerr<<"Error: "<<glewGetErrorString(err)<<endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
			cout<<"Driver supports OpenGL 3.3\nDetails:"<<endl;
		}
	}
	err = glGetError(); //this is to ignore INVALID ENUM error 1282
	GL_CHECK_ERRORS

	//print information on screen
	cout<<"\tUsing GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tVendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tVersion: "<<glGetString (GL_VERSION)<<endl;
	cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;

	GL_CHECK_ERRORS

	//initialization of OpenGL
	OnInit();

	//calback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutMouseWheelFunc(OnMouseWheel);
	glutIdleFunc(OnIdle);
		
	//main loop call
	glutMainLoop();
	
	return 0;
}