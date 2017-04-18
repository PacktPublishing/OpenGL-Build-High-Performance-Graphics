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
float rX=0, rY=0, fov = 45;

#include "..\..\src\FreeCamera.h"

//virtual key codes
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//for floating point imprecision
const float EPSILON = 0.001f;
const float EPSILON2 = EPSILON*EPSILON;

//delta time
float dt = 0;

//free camera instance
CFreeCamera cam;

//output message
#include <sstream>
std::stringstream msg;

//grid object
#include "..\..\src\Grid.h"
CGrid* grid;

//unit cube object
#include "..\..\src\UnitCube.h"
CUnitCube* cube;

//modelview and projection matrices
glm::mat4 MV,P;

//selected box index
int selected_box=-1;

//box positions
glm::vec3 box_positions[3]={glm::vec3(-1,0.5,0),
							glm::vec3(0,0.5,1),
							glm::vec3(1,0.5,0)
							};

//time related variables
float last_time=0, current_time=0;

//mouse filtering variables
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;

//mouse history buffer
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //filtered mouse values

//flag to enable filtering
bool useFiltering = true;

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

		//read pixel colour at mouse click position
		GLubyte pixel[4];
		glReadPixels(x, HEIGHT-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

		//based on colour decide which box was selected
		selected_box=-1;
		if(pixel[0]==255 && pixel[1]==0 && pixel[2]==0) {
			cout<<"picked box 1"<<endl;
			selected_box = 0;
		}
		if(pixel[0]==0 && pixel[1]==255 && pixel[2]==0) {
			cout<<"picked box 2"<<endl;
			selected_box = 1;
		}
		if(pixel[0]==0 && pixel[1]==0 && pixel[2]==255) {
			cout<<"picked box 3"<<endl;
			selected_box = 2;
		}
	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;
}


//mouse move handler
void OnMouseMove(int x, int y)
{
	if(selected_box == -1) {
		if (state == 0) {
			fov += (y - oldY)/5.0f;
			cam.SetupProjection(fov, cam.GetAspectRatio());
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
	}
	glutPostRedisplay();
}

//OpenGL initialization
void OnInit() {
	
	GL_CHECK_ERRORS

	//create a grid of size 20x20 in XZ plane
	grid = new CGrid(20,20);

	//create a unit cube
	cube = new CUnitCube();

	GL_CHECK_ERRORS

	//set the camera position
	glm::vec3 p = glm::vec3(10,10,10);
	cam.SetPosition(p);

	//get the camera look direction to obtain the yaw and pitch values for camera rotation
	glm::vec3 look=  glm::normalize(p);

	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;

	//if filtering is enabled, save positions to mouse history buffer
	if(useFiltering) {
		for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
			mouseHistory[i] = glm::vec2(rX, rY);
		}
	}
	cam.Rotate(rX,rY,0);

	//disbale dithering (requried for colour based picking since dithering might change the colours)
	glDisable(GL_DITHER);
	//enable depth test
	glEnable(GL_DEPTH_TEST);

	cout<<"Initialization successfull"<<endl;
}


//release all allocated resources
void OnShutdown() {

	delete grid;
	delete cube;
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport size
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//set the camera projection matrix
	cam.SetupProjection(fov, (GLfloat)w/h);
}

//idle callback function
void OnIdle() {

	//handle the WSAD, QZ key events to move the camera around
	if( GetAsyncKeyState(VK_W) & 0x8000) {
		cam.Walk(dt);
	}

	if( GetAsyncKeyState(VK_S) & 0x8000) {
		cam.Walk(-dt);
	}

	if( GetAsyncKeyState(VK_A) & 0x8000) {
		cam.Strafe(-dt);
	}

	if( GetAsyncKeyState(VK_D) & 0x8000) {
		cam.Strafe(dt);
	}

	if( GetAsyncKeyState(VK_Q) & 0x8000) {
		cam.Lift(dt);
	}

	if( GetAsyncKeyState(VK_Z) & 0x8000) {
		cam.Lift(-dt);
	}
	glm::vec3 t = cam.GetTranslation(); 
	if(glm::dot(t,t)>EPSILON2) {
		cam.SetTranslation(t*0.95f);
	}
	//call the display function
	glutPostRedisplay();
}


//display callback function
void OnRender() {
	GL_CHECK_ERRORS
	//timing related calcualtion
	last_time = current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	dt = current_time-last_time;

	//clear color buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the mesage
	msg.str(std::string());
	if(selected_box==-1)
		msg<<"No box picked";
	else
		msg<<"Picked box: "<<selected_box;

	//set the window title
	glutSetWindowTitle(msg.str().c_str());
	
	//set the camera modelview and projection matrices to get the combined MVP matrix
	MV	= cam.GetViewMatrix();
	P   = cam.GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

	//render the grid object
	grid->Render(glm::value_ptr(MVP));

	//set the first cube transform 
	//set its colour to cyan if selected, red otherwise
	glm::mat4 T = glm::translate(glm::mat4(1), box_positions[0]);
	cube->color = (selected_box==0)?glm::vec3(0,1,1):glm::vec3(1,0,0);
	cube->Render(glm::value_ptr(MVP*T));

	//set the second cube transform 
	//set its colour to cyan if selected, green otherwise
	T = glm::translate(glm::mat4(1), box_positions[1]);
	cube->color = (selected_box==1)?glm::vec3(0,1,1):glm::vec3(0,1,0);
	cube->Render(glm::value_ptr(MVP*T));

	//set the third cube transform 
	//set its colour to cyan if selected, blue otherwise
	T = glm::translate(glm::mat4(1), box_positions[2]);
	cube->color = (selected_box==2)?glm::vec3(0,1,1):glm::vec3(0,0,1);
	cube->Render(glm::value_ptr(MVP*T));

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
	glutCreateWindow("Picking using color picking - OpenGL 3.3");

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