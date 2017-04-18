#define _USE_MATH_DEFINES
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

//screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//camera tranformation variables
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, dist = 10;

#include "..\..\src\TargetCamera.h"

//virtual key codes
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//delta time
float dt = 0;

//timing related variables
float last_time=0, current_time =0;

const float MOVE_SPEED = 5; //m/s

//target camera instance
CTargetCamera cam;

//mouse filtering support variables
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;

//mouse history buffer
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //filtered mouse values

//flag to enable filtering
bool useFiltering = true;

//output message
#include <sstream>
std::stringstream msg;

//floor checker texture ID
GLuint checkerTextureID;


//checkered plane object
#include "..\..\src\TexturedPlane.h"
CTexturedPlane* checker_plane;

//mouse move filtering function
void filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    // Store current mouse entry at front of array.
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    // Filter the mouse.
    for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE; ++i)
    {
		glm::vec2 tmp=mouseHistory[i];
        averageX += tmp.x * currentWeight;
        averageY += tmp.y * currentWeight;
        averageTotal += 1.0f * currentWeight;
        currentWeight *= MOUSE_FILTER_WEIGHT;
    }

    mouseX = averageX / averageTotal;
    mouseY = averageY / averageTotal;

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
	if (state == 0) {
		dist = (y - oldY)/5.0f;
		cam.Zoom(dist);
	} else if(state ==2) {
		float dy = float(y-oldY)/100.0f;
		float dx = float(oldX-x)/100.0f;
		if(useFiltering)
			filterMouseMoves(dx, dy);
		else {
			mouseX = dx;
			mouseY = dy;
		}

		cam.Pan(mouseX, mouseY);
	} else {
		rY += (y - oldY)/5.0f;
		rX += (oldX-x)/5.0f;
		if(useFiltering)
			filterMouseMoves(rX, rY);
		else {
			mouseX = rX;
			mouseY = rY;
		}
		cam.Rotate(mouseX,mouseY, 0);
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//initialize OpenGL
void OnInit() {
	GL_CHECK_ERRORS
	//generate the checker texture
	GLubyte data[128][128]={0};
	for(int j=0;j<128;j++) {
		for(int i=0;i<128;i++) {
			data[i][j]=(i<=64 && j<=64 || i>64 && j>64 )?255:0;
		}
	}

	//generate texture object
	glGenTextures(1, &checkerTextureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, checkerTextureID);
	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GL_CHECK_ERRORS

	//set maximum aniostropy setting
	GLfloat largest_supported_anisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);

	//set mipmap base and max level
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);

	//allocate texture object
	glTexImage2D(GL_TEXTURE_2D,0,GL_RED, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE, data);

	//generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	GL_CHECK_ERRORS
		
	//create a textured plane object
	checker_plane = new CTexturedPlane();

	GL_CHECK_ERRORS
		
	//setup the camera position and target
	cam.SetPosition(glm::vec3(5,5,5));
	cam.SetTarget(glm::vec3(0,0,0));

	//also rotate the camera for proper orientation
	glm::vec3 look =  glm::normalize(cam.GetTarget()-cam.GetPosition());

	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;

	cam.Rotate(rX,rY,0);
	cout<<"Initialization successfull"<<endl;
}

//delete all allocated resources
void OnShutdown() { 
	delete checker_plane;
	glDeleteTextures(1, &checkerTextureID);
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the camera projection matrix
	cam.SetupProjection(45, (GLfloat)w/h); 
}

//idle event processing
void OnIdle() {
	bool bPressed = false;
	float dx=0, dy=0;
	//handle the WSAD, QZ key events to move the camera around
	if( GetAsyncKeyState(VK_W) & 0x8000) {
		dy += (MOVE_SPEED*dt);
		bPressed = true;
	}

	if( GetAsyncKeyState(VK_S) & 0x8000) {
		dy -= (MOVE_SPEED*dt);
		bPressed = true;
	} 

	if( GetAsyncKeyState(VK_A) & 0x8000) {
		dx -= (MOVE_SPEED*dt);
		bPressed = true;
	}

	if( GetAsyncKeyState(VK_D) & 0x8000) {
		dx += (MOVE_SPEED*dt);
		bPressed = true;
	}

	if(bPressed)
		cam.Move(dx, dy);

	//call the display function
	glutPostRedisplay();
}

//display callback function
void OnRender() {
	//timing related calcualtion
	last_time = current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	dt = current_time-last_time;

	//clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transformation
	glm::mat4 MV	= cam.GetViewMatrix();
	glm::mat4 P     = cam.GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

	//render the chekered plane
	checker_plane->Render(glm::value_ptr(MVP));

	//swap front and back buffers to show the rendered result
	glutSwapBuffers(); 
}

//Keyboard event handler to toggle the mouse filtering using spacebar key
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ' ':
			useFiltering = !useFiltering;
		break;
	}
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
	glutCreateWindow("Target Camera - OpenGL 3.3");

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
	glutKeyboardFunc(OnKey);
	glutIdleFunc(OnIdle);

	//call main loop
	glutMainLoop();

	return 0;
}