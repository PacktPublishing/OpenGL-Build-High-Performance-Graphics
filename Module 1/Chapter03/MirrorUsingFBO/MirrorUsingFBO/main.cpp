#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;
 
//Grid object
#include "..\..\src\Grid.h"
CGrid* grid;

//Unit colour cube
#include "..\..\src\UnitColorCube.h"
CUnitColorCube* cube;

//Quad
#include "..\..\src\Quad.h"
CQuad* mirror;

//projection and modelview matrix
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -7;

//FBO and render buffer object ID
GLuint fboID, rbID;

//offscreen render texture ID
GLuint renderTextureID;

//local rotation matrix
glm::mat4 localR=glm::mat4(1);

//autorotate angle
float angle = 0;

//initialize FBO
void InitFBO() {
	//generate and bind fbo ID
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

	//generate and bind render buffer ID
	glGenRenderbuffers(1, &rbID);
	glBindRenderbuffer(GL_RENDERBUFFER, rbID);

	//set the render buffer storage
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, WIDTH, HEIGHT);

	//generate the offscreen texture
	glGenTextures(1, &renderTextureID);
	glBindTexture(GL_TEXTURE_2D, renderTextureID);

	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	//bind the renderTextureID as colour attachment of FBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextureID, 0);
	//set the render buffer as the depth attachment of FBO
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER, rbID);

	//check for frame buffer completeness status
	GLuint status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if(status==GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO setup succeeded.");
	} else {
		printf("Error in FBO setup.");
	}

	//unbind the texture and FBO
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

//release allocated FBO resources
void ShutdownFBO() {
	glDeleteTextures(1, &renderTextureID);
	glDeleteRenderbuffers(1, &rbID);
	glDeleteFramebuffers(1, &fboID);
}

//mouse click event handler
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

//mouse move event handler
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

	glEnable(GL_DEPTH_TEST);

	GL_CHECK_ERRORS

	//create the 20x20 grid in XZ plane
	grid = new CGrid(20,20);

	//create a unit colour cube
	cube = new CUnitColorCube();

	//create a quad as mirror object at Z=-2 position
	mirror = new CQuad(-2);

	//initialize FBO object
	InitFBO();

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	ShutdownFBO();

	delete grid;
	delete cube;
	delete mirror;

	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport size
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(45.0f, (GLfloat)w/h, 1.f, 1000.f);
}

//idle event handler
void OnIdle() {
	//increment angle and create a local rotation matrix on Y axis
	angle += 0.5f; 
	localR=glm::rotate(glm::mat4(1), angle, glm::vec3(0,1,0));

	//call the display function
	glutPostRedisplay();
}

//display callback 
void OnRender() {

	//set the camera transformation 
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 Ry	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 MV	= Ry;
	glm::mat4 MVP	= P*MV;

	//clear the colour and depth buffers 
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//render scene normally	
	//render the grid
	grid->Render(glm::value_ptr(MVP));
	localR[3][1] = 0.5;
	
	//move the unit cube on Y axis to bring it to ground level
	//and render the cube
	cube->Render(glm::value_ptr(P*MV*localR));

	//store the current modelview matrix
	glm::mat4 oldMV = MV;

	//now change the view matrix to where the mirror is
	//reflect the view vector in the mirror normal direction
	glm::vec3 V = glm::vec3(-MV[2][0], -MV[2][1], -MV[2][2]);
	glm::vec3 R = glm::reflect(V, mirror->normal);

	//place the virtual camera at the mirro position
	MV = glm::lookAt(mirror->position, mirror->position + R, glm::vec3(0,1,0));

	//since mirror image is laterally inverted, we multiply the MV matrix by (-1,1,1)
	MV = glm::scale(MV, glm::vec3(-1,1,1));

	//enable FBO 
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
	//render to colour attachment 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//clear the colour and depth buffers
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//show the mirror from the front side only
		if(glm::dot(V,mirror->normal)<0) {
			grid->Render(glm::value_ptr(P*MV));
			cube->Render(glm::value_ptr(P*MV*localR));
		} 
	
	//unbind the FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	//restore the default back buffer 
	glDrawBuffer(GL_BACK_LEFT);

	//reset the old modelview matrix
	MV = oldMV;

	//bind the FBO output at the current texture 
	glBindTexture(GL_TEXTURE_2D, renderTextureID);

	//render mirror
	mirror->Render(glm::value_ptr(P*MV));

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
	glutCreateWindow("Mirror using FBO - OpenGL 3.3");

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

	//opengl initialization
	OnInit();

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutIdleFunc(OnIdle);

	//call main loop
	glutMainLoop();

	return 0;
}