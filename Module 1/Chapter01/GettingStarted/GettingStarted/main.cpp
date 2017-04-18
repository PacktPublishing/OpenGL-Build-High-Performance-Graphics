#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#pragma comment(lib, "glew32.lib")

using namespace std;

//screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//OpenGL initialization 
void OnInit() {
	//set clear color to red
	glClearColor(1,0,0,0);
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	cout<<"Shutdown successfull"<<endl;
}

//handle resize event
void OnResize(int nw, int nh) {

}

//display callback function
void OnRender() {
	//clear colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Getting started with OpenGL 3.3");

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

	//print information on screen
	cout<<"\tUsing GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tVendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tVersion: "<<glGetString (GL_VERSION)<<endl;
	cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;

	//initialization of OpenGL
	OnInit();
		
	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);

	//main loop call
	glutMainLoop();

	return 0;
}