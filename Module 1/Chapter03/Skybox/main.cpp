

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"

#include <SOIL.h>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;

//projection and modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=-3, rY=65, dist = -7;

//skybox object
#include "../src/skybox.h"
CSkybox* skybox;

//skybox texture ID
GLuint skyboxTextureID;

//skybox texture names
const char* texture_names[6] = {"../media/skybox/ocean/posx.png",
								"../media/skybox/ocean/negx.png",
								"../media/skybox/ocean/posy.png",
								"../media/skybox/ocean/negy.png",
								"../media/skybox/ocean/posz.png",
								"../media/skybox/ocean/negz.png"};
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

	//enable depth testing and culling 
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	GL_CHECK_ERRORS

	//generate a new Skybox
	skybox = new CSkybox();

	GL_CHECK_ERRORS
		
	//load skybox textures using SOIL
	int texture_widths[6];
	int texture_heights[6];
	int channels[6];
	GLubyte* pData[6];

	cout<<"Loading skybox images: ..."<<endl;
	for(int i=0;i<6;i++) {
		cout<<"\tLoading: "<<texture_names[i]<<" ... ";
		pData[i] = SOIL_load_image(texture_names[i],	&texture_widths[i], &texture_heights[i], &channels[i], SOIL_LOAD_AUTO);
		cout<<"done."<<endl;
	}

	GL_CHECK_ERRORS

	//generate OpenGL texture
    glGenTextures(1, &skyboxTextureID);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

	GL_CHECK_ERRORS
	//set texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GL_CHECK_ERRORS

	//set the image format
	GLint format = (channels[0]==4)?GL_RGBA:GL_RGB;
    //load the 6 images
	for(int i=0;i<6;i++) {
		//allocate cubemap data
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, texture_widths[i], texture_heights[i], 0, format, GL_UNSIGNED_BYTE, pData[i]);

		//free SOIL image data
		SOIL_free_image_data(pData[i]);
	}

	GL_CHECK_ERRORS
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	delete skybox;

	glDeleteTextures(1, &skyboxTextureID);
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.1f, 1000.f);
}

//idle event handler
void OnIdle() {
	glutPostRedisplay();
}

//display callback function
void OnRender() {
	GL_CHECK_ERRORS

	//clear colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transform
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(glm::mat4(1),  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 S     = glm::scale(glm::mat4(1),glm::vec3(1000.0));
    glm::mat4 MVP	= P*MV*S;

	//render the skybox object
	skybox->Render( glm::value_ptr(MVP));

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
	glutCreateWindow("Skybox - OpenGL 3.3");

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

	//call main loop
	glutMainLoop();

	return 0;
}