

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

//screen resolution
const int WIDTH  = 1024;
const int HEIGHT = 768;

//cubemap size
const int CUBEMAP_SIZE = 1024;

//shaders for rendering and cubemap generation
GLSLShader shader, cubemapShader;

//vertex struct for storing per-vertex position and normal
struct Vertex {
	glm::vec3 pos, normal;
};

//sphere vertex array and vertex buffer objects
GLuint sphereVAOID;
GLuint sphereVerticesVBO;
GLuint sphereIndicesVBO;

//distance to radially displace the spheres
float radius = 2;

//projection, cubemap, modelview and rotation matrices
glm::mat4  P = glm::mat4(1);
glm::mat4  Pcubemap = glm::mat4(1);
glm::mat4  MV = glm::mat4(1);
glm::mat4 Rot;

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -10;

//dynamic cubemap texture ID
GLuint dynamicCubeMapID;

//FBO and RBO IDs
GLuint fboID, rboID;


//grid object
#include "../src/Grid.h"
CGrid* grid;


//unit cube object
#include "../src/UnitCube.h"
CUnitCube* cube;

//eye position
glm::vec3 eyePos;

#include <vector>
using namespace std;

//vertice and indices of geometry
std::vector<Vertex> vertices;
std::vector<GLushort> indices;

//autorotate angle
float angle = 0; 

float dx=-0.1f;	//direction 
 

//constant colours array
const glm::vec3 colors[8]={glm::vec3(1,0,0),glm::vec3(0,1,0),glm::vec3(0,0,1),glm::vec3(1,1,0),glm::vec3(1,0,1),glm::vec3(0,1,1),glm::vec3(1,1,1),glm::vec3(0.5,0.5,0.5)};

//adds the given sphere indices to the indices vector
inline void push_indices(int sectors, int r, int s) {
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
void createSphere( float radius, unsigned int slices, unsigned int stacks)
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
            push_indices(stacks, r, s);
        }
    }

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
	else
		state = 1;
}

//mouse move handler
void OnMouseMove(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f);
	else
	{
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//OpenGL initialization
void OnInit() { 

	//load the cubemap shader
	cubemapShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/cubemap.vert");
	cubemapShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/cubemap.frag");
	//compile and link shader
	cubemapShader.CreateAndLinkProgram();
	cubemapShader.Use();
		//add shader attribute and uniforms
		cubemapShader.AddAttribute("vVertex");
		cubemapShader.AddAttribute("vNormal");
		cubemapShader.AddUniform("MVP");
		cubemapShader.AddUniform("eyePosition");
		cubemapShader.AddUniform("cubeMap");
		//set values of constant uniforms at initialization
		glUniform1i(cubemapShader("cubeMap"), 1);
	cubemapShader.UnUse();

	GL_CHECK_ERRORS

	//setup sphere geometry
	createSphere(1,10,10);

	GL_CHECK_ERRORS

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
		 
	//generate the dynamic cubemap texture and bind to texture unit 1
	glGenTextures(1, &dynamicCubeMapID);
	glActiveTexture(GL_TEXTURE1);	
	glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicCubeMapID);
	//set texture parameters
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//for all 6 cubemap faces
	for (int face = 0; face < 6; face++) {
		//allocate a different texture for each face and assign to the cubemap texture target
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, CUBEMAP_SIZE, CUBEMAP_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	GL_CHECK_ERRORS

	//setup FBO
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

	//setup render buffer object (RBO)
	glGenRenderbuffers(1, &rboID);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID);

	//set the renderbuffer storage to have the same dimensions as the cubemap texture
	//also set the renderbuffer as the depth attachment of the FBO
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, CUBEMAP_SIZE, CUBEMAP_SIZE);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fboID);

	//set the dynamic cubemap texture as the colour attachment of FBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, dynamicCubeMapID, 0);

	//check the framebuffer completeness status
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		cerr<<"Frame buffer object setup error."<<endl;
		exit(EXIT_FAILURE);
	} else {
		cerr<<"FBO setup successfully."<<endl;
	}
	//unbind FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//unbind renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GL_CHECK_ERRORS

	//create a grid object
	grid = new CGrid();

	//create a unit cube object
	cube = new CUnitCube(glm::vec3(1,0,0));

	GL_CHECK_ERRORS

	//enable depth testing and back face culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	//Destroy shader
	shader.DeleteShaderProgram();
	cubemapShader.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1, &sphereVerticesVBO);
	glDeleteBuffers(1, &sphereIndicesVBO);
	glDeleteVertexArrays(1, &sphereVAOID);

	delete grid;
	delete cube;

	glDeleteTextures(1, &dynamicCubeMapID);


	glDeleteFramebuffers(1, &fboID);
	glDeleteRenderbuffers(1, &rboID);
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport size
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.1f, 1000.f);
	//setup the cube map projection matrix
	Pcubemap = glm::perspective(90.0f,1.0f,0.1f,1000.0f);
}

//idle event callback
void OnIdle() {
	//generate a new Y rotation matrix
	Rot = glm::rotate(glm::mat4(1), angle++, glm::vec3(0,1,0));

	//call display function
	glutPostRedisplay();
}

//scene rendering function
void DrawScene(glm::mat4 MView, glm::mat4 Proj ) {

	//for each cube
	for(int i=0;i<8;i++){
		//determine the cube's transform
		float angle = (float)(i/8.0f*2*M_PI);
		glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(radius*cos(angle),0.5,radius*sin(angle)));

		//get the combined modelview projection matrix
		glm::mat4 MVP = Proj*MView*Rot*T;

		//set the cube's colour
		cube->color = colors[i];

		//render the cube
		cube->Render(glm::value_ptr(MVP));
	}

	//render the grid object
	grid->Render(glm::value_ptr(Proj*MView));
}

//display callback function
void OnRender() {

	//increment the radius
	radius+=dx;

	//if radius is beyond limits, invert the movement direction
	if(radius<1 || radius>5)
		dx=-dx;

	GL_CHECK_ERRORS

	//clear colour buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transform
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//get the eye position
	eyePos.x = -(MV[0][0] * MV[3][0] + MV[0][1] * MV[3][1] + MV[0][2] * MV[3][2]);
    eyePos.y = -(MV[1][0] * MV[3][0] + MV[1][1] * MV[3][1] + MV[1][2] * MV[3][2]);
    eyePos.z = -(MV[2][0] * MV[3][0] + MV[2][1] * MV[3][1] + MV[2][2] * MV[3][2]);

	//p is to translate the sphere to bring it to the ground level
	glm::vec3 p=glm::vec3(0,1,0);

	//when rendering to the CUBEMAP texture, we move all of cubes by opposite amount
	//so that the projection is clearly visible
	T = glm::translate(glm::mat4(1), -p);
	 
	//set the viewport to the size of the cube map texture
	glViewport(0,0,CUBEMAP_SIZE,CUBEMAP_SIZE);

	//bind the FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

		//set the GL_TEXTURE_CUBE_MAP_POSITIVE_X to the colour attachment of FBO
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, dynamicCubeMapID, 0);
		//clear the colour and depth buffers
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//set the virtual viewrer at the reflective object center and render the scene
		//using the cube map projection matrix and appropriate viewing settings
		glm::mat4 MV1 = glm::lookAt(glm::vec3(0),glm::vec3(1,0,0), glm::vec3(0,-1,0));
 		DrawScene( MV1*T, Pcubemap);

		//set the GL_TEXTURE_CUBE_MAP_NEGATIVE_X to the colour attachment of FBO
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, dynamicCubeMapID, 0);
		//clear the colour and depth buffers
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//set the virtual viewrer at the reflective object center and render the scene
		//using the cube map projection matrix and appropriate viewing settings
		glm::mat4 MV2 = glm::lookAt(glm::vec3(0),glm::vec3(-1,0,0), glm::vec3(0,-1,0));
		DrawScene( MV2*T, Pcubemap);
		
		//set the GL_TEXTURE_CUBE_MAP_POSITIVE_Y to the colour attachment of FBO		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, dynamicCubeMapID, 0);
		//clear the colour and depth buffers 
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//set the virtual viewrer at the reflective object center and render the scene
		//using the cube map projection matrix and appropriate viewing settings
		glm::mat4 MV3 = glm::lookAt(glm::vec3(0),glm::vec3(0,1,0), glm::vec3(1,0,0));
		DrawScene( MV3*T, Pcubemap);

		//set the GL_TEXTURE_CUBE_MAP_NEGATIVE_Y to the colour attachment of FBO		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, dynamicCubeMapID, 0);
		//clear the colour and depth buffers
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//set the virtual viewrer at the reflective object center and render the scene
		//using the cube map projection matrix and appropriate viewing settings
		glm::mat4 MV4 = glm::lookAt(glm::vec3(0),glm::vec3(0,-1,0), glm::vec3(1,0,0));
		DrawScene( MV4*T, Pcubemap);

		//set the GL_TEXTURE_CUBE_MAP_POSITIVE_Z to the colour attachment of FBO		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, dynamicCubeMapID, 0);
		//clear the colour and depth buffers
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//set the virtual viewrer at the reflective object center and render the scene
		//using the cube map projection matrix and appropriate viewing settings
		glm::mat4 MV5 = glm::lookAt(glm::vec3(0),glm::vec3(0,0,1), glm::vec3(0,-1,0));
		DrawScene(MV5*T, Pcubemap);

		//set the GL_TEXTURE_CUBE_MAP_NEGATIVE_Z to the colour attachment of FBO		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, dynamicCubeMapID, 0);
		//clear the colour and depth buffers
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//set the virtual viewrer at the reflective object center and render the scene
		//using the cube map projection matrix and appropriate viewing settings
		glm::mat4 MV6 = glm::lookAt(glm::vec3(0),glm::vec3(0,0,-1), glm::vec3(0,-1,0));
		DrawScene( MV6*T, Pcubemap);

	//unbind the FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//reset the default viewport 
	glViewport(0,0,WIDTH,HEIGHT);

	//render scene from the camera point of view and projection matrix
	DrawScene(MV, P);

	//bind the sphere vertex array object
	glBindVertexArray(sphereVAOID);

	//use the cubemap shader to render the reflective sphere
	cubemapShader.Use();
		//set the sphere transform
		T = glm::translate(glm::mat4(1), p);
		//set the shader uniforms
		glUniformMatrix4fv(cubemapShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*(MV*T)));
		glUniform3fv(cubemapShader("eyePosition"), 1,  glm::value_ptr(eyePos));
			//draw the sphere triangles
			glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_SHORT,0);

	//unbind shader
	cubemapShader.UnUse();

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}
 

int main(int argc, char** argv) {
	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Dynamic Cubemapping - OpenGL 3.3");

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
	glutIdleFunc(OnIdle);

	//main loop call
	glutMainLoop();

	return 0;
}