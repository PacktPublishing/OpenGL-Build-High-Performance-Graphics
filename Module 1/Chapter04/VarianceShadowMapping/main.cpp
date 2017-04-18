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
const int SHADOWMAP_WIDTH = 256;
const int SHADOWMAP_HEIGHT = SHADOWMAP_WIDTH;

//variance shadowmapping shaders
GLSLShader shader;				//variance shadow mapping main shader
GLSLShader firstStep;			//first step shader for outputting the moments
GLSLShader flatshader;			//shader for rendering of light gizmo
GLSLShader gaussianH_shader;	//horizontal Gaussian smoothing shader
GLSLShader gaussianV_shader;	//vertical Gaussian smoothing shader

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

//fullscreen quad vertex array and vertex buffer objects
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

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
float radius = 7.5f;

//shadow map texture ID
GLuint shadowMapTexID;

//FBO and render buffer object IDs
GLuint fboID, rboID;

//filtering FBO ID
GLuint filterFBOID;
//filtering FBO colour attachment texture 
GLuint blurTexID[2];

glm::mat4 MV_L; //light modelview matrix
glm::mat4 P_L;	//light projection matrix
glm::mat4 B;    //light bias matrix
glm::mat4 BP;   //light bias and projection matrix combined
glm::mat4 S;    //light's combined MVPB matrix

//ping pong IDs
int readID =0, writeID=1;

//coour attachment array
GLenum attachID[2]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

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

		//update the light position
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

	//call display function
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

	GL_CHECK_ERRORS

	//load the horizontal Gaussian smoothing shader
	gaussianH_shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Passthrough.vert");
	gaussianH_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/GaussH.frag");
	//compile and link shader
	gaussianH_shader.CreateAndLinkProgram();
	gaussianH_shader.Use();
		//add attributes and uniforms
		gaussianH_shader.AddAttribute("vVertex");
		gaussianH_shader.AddUniform("textureMap");
		//pass value of constant uniforms at initialization
		glUniform1i(gaussianH_shader("textureMap"),1);
	gaussianH_shader.UnUse();

	GL_CHECK_ERRORS

	//load the vertical Gaussian smoothing shader
	gaussianV_shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Passthrough.vert");
	gaussianV_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/GaussV.frag");
	//compile and link shader
	gaussianV_shader.CreateAndLinkProgram();
	gaussianV_shader.Use();
		//add attributes and uniforms
		gaussianV_shader.AddAttribute("vVertex");
		gaussianV_shader.AddUniform("textureMap");
		//pass value of constant uniforms at initialization
		glUniform1i(gaussianV_shader("textureMap"),0);
	gaussianV_shader.UnUse();

	GL_CHECK_ERRORS

	//load the variance shadow mapping first step shader
	firstStep.LoadFromFile(GL_VERTEX_SHADER, "shaders/firstStep.vert");
	firstStep.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/firstStep.frag");
	//compile and link shader
	firstStep.CreateAndLinkProgram();
	firstStep.Use();
		//add attributes and uniforms
		firstStep.AddAttribute("vVertex");
		firstStep.AddUniform("MVP");
	firstStep.UnUse();

	GL_CHECK_ERRORS

	//load the variance shadow mapping shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/VarianceShadowMapping.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/VarianceShadowMapping.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MVP");
		shader.AddUniform("MV");
		shader.AddUniform("M");
		shader.AddUniform("N");
		shader.AddUniform("S");
		shader.AddUniform("light_position");
		shader.AddUniform("diffuse_color");
		shader.AddUniform("shadowMap");
		//pass value of constant uniforms at initialization
		glUniform1i(shader("shadowMap"),2);
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
		//enable vertex attribute array for normal
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

	//clear the vertices and indices vectors as we will reuse them
	//for plane
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

	//fullscreen quad vertices
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//pass quad indices
	indices.clear();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	//generate quad vertex array and vertex buffer objects
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//pass quad vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);
		//pass quad indices to element array buffer
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

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

	//set up FBO and render buffer object (RBO) to get the depth component
	glGenFramebuffers(1,&fboID);
	glGenRenderbuffers(1, &rboID);

	//bind the FBO and RBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID);

	//set the shadow map resolution for the render buffer storage
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	
	//set the border colour 
	GLfloat border[4]={1,0,0,0};
	
	//generate OpenGL texture on texture unit 0
	glGenTextures(1, &shadowMapTexID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexID);
		//set the texture parameters
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,SHADOWMAP_WIDTH,SHADOWMAP_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);

		//enable mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		glGenerateMipmap(GL_TEXTURE_2D);

	//set the shadow map texture as colour attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,shadowMapTexID,0);
	//set the render buffer as the depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);

	//check the framebuffer completeness status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"FBO setup successful."<<endl;
	} else {
		cout<<"Problem in FBO setup."<<endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER,0);

	//setup filtering fbo
	glGenFramebuffers(1,&filterFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);

	//generate two colour attachments on texture units 1 and 2
	glGenTextures(2, blurTexID);
	for(int i=0;i<2;i++) {
		//set texture parameters
		glActiveTexture(GL_TEXTURE1+i);
		glBindTexture(GL_TEXTURE_2D, blurTexID[i]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
		//allocate texture object
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,SHADOWMAP_WIDTH,SHADOWMAP_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);
		//add to FBO colour attachment 0 and 1
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,blurTexID[i],0);
	}
	//check the framebuffer completeness status
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Filtering FBO setup successful."<<endl;
	} else {
		cout<<"Problem in Filtering FBO setup."<<endl;
	}

	//unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	//set the light MV, P and bias matrices
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	P_L  = glm::perspective(50.0f,1.0f,1.0f, 50.0f);
	B    = glm::scale(glm::translate(glm::mat4(1),glm::vec3(0.5,0.5,0.5)), glm::vec3(0.5,0.5,0.5));
	BP   = B*P_L;
	S    = BP*MV_L;

	//enable depth testing and culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {

	glDeleteTextures(1, &shadowMapTexID);
	glDeleteTextures(2, blurTexID);

	//Destroy shaders
	shader.DeleteShaderProgram();
	flatshader.DeleteShaderProgram();
	firstStep.DeleteShaderProgram();
	gaussianH_shader.DeleteShaderProgram();
	gaussianV_shader.DeleteShaderProgram();

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

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);

	glDeleteFramebuffers(1, &fboID);
	glDeleteFramebuffers(1, &filterFBOID);
	glDeleteRenderbuffers(1, &rboID);

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

//scene rendering function for first pass
void DrawSceneFirstPass(glm::mat4 View, glm::mat4 Proj) {

	GL_CHECK_ERRORS

	//bind the first step shader
	firstStep.Use();
		//bind the plane VAO
		glBindVertexArray(planeVAOID); {
			//set shader uniforms
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(Proj*View));
				//render the plane triangles
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

		//draw the cube object
		//bind the cube VAO
		glBindVertexArray(cubeVAOID); {
			//set the cube's transform
			glm::mat4 T = glm::translate(glm::mat4(1),  glm::vec3(-1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
			//set the shader uniform
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
				//render the cube triangles
	 			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		}

		//draw the sphere
		//bind the sphere VAO
		glBindVertexArray(sphereVAOID); {
			//set the sphere's transform
			glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
			//set the shader uniform
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
				//draw the sphere triangles
		 		glDrawElements(GL_TRIANGLES, totalSphereTriangles, GL_UNSIGNED_SHORT, 0);
		}

	//unbind the first step shader
	firstStep.UnUse();

	GL_CHECK_ERRORS
}

//scene rendering for final pass
void DrawScene(glm::mat4 View, glm::mat4 Proj ) {

	GL_CHECK_ERRORS

	//bind the variance shadow mapping shader
	shader.Use();
		

		//bind the plane VAO
		glBindVertexArray(planeVAOID); {
			//pass the shader uniforms 
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x));
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(Proj*View));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(View));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(View))));
			glUniform3f(shader("diffuse_color"), 1.0f,1.0f,1.0f);
				//render plane triangles
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

		//draw the cube 
		//bind teh cube VAO
		glBindVertexArray(cubeVAOID); {
			//set the cube's transform
			glm::mat4 T = glm::translate(glm::mat4(1),  glm::vec3(-1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
			//pass the shader uniforms
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(M));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniform3f(shader("diffuse_color"), 1.0f,0.0f,0.0f);
				//render cube's triangles
	 			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		}

		//bind the sphere VAO
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
				//render sphere triangles
		 		glDrawElements(GL_TRIANGLES, totalSphereTriangles, GL_UNSIGNED_SHORT, 0);
		}
	
	//unbind the shader
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
	  
	//1) Render scene from the light's POV
	//enable rendering to FBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	//reset viewport to the shadow map texture size
	glViewport(0,0,SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
		//set drawing to colour attachment 0
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//clear the colour and depth buffers
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			//draw scene using the first pass shader from the point of view of light
			DrawSceneFirstPass(MV_L, P_L);

	//bind the filtering FBO 
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);
	//set drawing to colour attachment 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//bind the fullscreen quad VAO
	glBindVertexArray(quadVAOID);
		//use the vertical Gaussian smoothing shader
		gaussianV_shader.Use();
			//render quad triangles
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	//set drawing to colour attachment 1
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
		//use the horizontal Gaussian smoothing shader
		gaussianH_shader.Use();
			//render quad triangles
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	//unbind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//restore drawing to back buffer
	glDrawBuffer(GL_BACK_LEFT);
	//restore the viewport to the screen size
	glViewport(0,0,WIDTH, HEIGHT);
		//render scene normally
		DrawScene(MV, P); 
		  
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
	glutCreateWindow("Variance Shadow Mapping - OpenGL 3.3");

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
	
	//callback hooks
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