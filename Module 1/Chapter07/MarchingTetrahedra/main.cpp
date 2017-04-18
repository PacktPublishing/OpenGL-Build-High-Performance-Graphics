#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"
#include <fstream>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);


#pragma comment(lib, "glew32.lib")

using namespace std;

//set screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//camera transform variables
int state = 0, oldX=0, oldY=0;
float rX=4, rY=50, dist = -2;

//grid object
#include "..\src\Grid.h"
CGrid* grid;

//modelview and projection matrices
glm::mat4 MV,P;
 
//flag to set wireframe rendering mode
bool bWireframe = false;

//volume marcher vertex array and vertex buffer object IDs
GLuint volumeMarcherVBO;
GLuint volumeMarcherVAO;

//shader
GLSLShader shader;

//background colour
glm::vec4 bg=glm::vec4(0.5,0.5,1,1);

//volume filename
const std::string volume_file = "../media/Engine256.raw";
 
//TetrahedraMarcher instance
#include "TetrahedraMarcher.h"
TetrahedraMarcher* marcher;

//mouse down event handler
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
	if (state == 0) {
		dist += (y - oldY)/50.0f;
	} else {
		rX += (y - oldY)/5.0f;
		rY += (x - oldX)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}
 
//OpenGL initialization function
void OnInit() {

	GL_CHECK_ERRORS

	//create a uniform grid of size 20x20 in XZ plane
	grid = new CGrid(20,20);

	GL_CHECK_ERRORS

	//create a new TetrahedraMarcher instance
	marcher = new TetrahedraMarcher();
	//set the volume dataset dimensions
	marcher->SetVolumeDimensions(256,256,256);
	//load the volume dataset
	marcher->LoadVolume(volume_file);
	//set the isosurface value
	marcher->SetIsosurfaceValue(48);
	//set the number of sampling voxels 
	marcher->SetNumSamplingVoxels(128,128,128);
	//begin tetrahedra marching
	marcher->MarchVolume();

	//setup the volume marcher vertex array object and vertex buffer object
	glGenVertexArrays(1, &volumeMarcherVAO);
	glGenBuffers(1, &volumeMarcherVBO);
	glBindVertexArray(volumeMarcherVAO);
	glBindBuffer (GL_ARRAY_BUFFER, volumeMarcherVBO);

	//pass the obtained vertices from the tetrahedra marcher and pass to the
	//buffer object memory
	glBufferData (GL_ARRAY_BUFFER, marcher->GetTotalVertices()*sizeof(Vertex), marcher->GetVertexPointer(), GL_STATIC_DRAW);

	//enable vertex attribute array for position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);

	//enable vertex attribute array for normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),(const GLvoid*)offsetof(Vertex, normal));

	GL_CHECK_ERRORS

	//load the shader 
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/marcher.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/marcher.frag");

	//compile and link the shader program
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attribute and uniform
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MVP");
	shader.UnUse();

	GL_CHECK_ERRORS

	//set the background colour
	glClearColor(bg.r, bg.g, bg.b, bg.a);

	//enable depth test and culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	shader.DeleteShaderProgram();
	glDeleteVertexArrays(1, &volumeMarcherVAO);
	glDeleteBuffers(1, &volumeMarcherVBO);

	delete grid;
	delete marcher;
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}

//display callback function
void OnRender() {
	GL_CHECK_ERRORS
	//set the camera transform
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//get the combined modelview projection matrix
    glm::mat4 MVP	= P*MV;

	//render the grid object
	grid->Render(glm::value_ptr(MVP));

	//set the modelling transform to move the marhing result to origin
	glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(-0.5,-0.5,-0.5));

	//if rendering mode set to wireframe we set the front and back 
	//polygon mode to line
	if(bWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	//set the volume marcher vertex array object
	glBindVertexArray(volumeMarcherVAO);
		//bind the shader
		shader.Use();
			//set the shader uniforms
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP*T));
				//render the triangles
				glDrawArrays(GL_TRIANGLES, 0, marcher->GetTotalVertices());
		//unbind the shader
		shader.UnUse();
	
	//restore the default polygon mode
	if(bWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//keyboard function to change the wireframe rendering mode
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case 'w': 	bWireframe = !bWireframe;	break; 
	}
	//recall display function
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Iso-surface extraction using Marching Tetrahedra - OpenGL 3.3");

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

	//output hardware information
	cout<<"\tUsing GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tVendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tVersion: "<<glGetString (GL_VERSION)<<endl;
	cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;

	GL_CHECK_ERRORS

	//OpenGL initialization
	OnInit();

	//attach callbacks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(OnKey);

	//run main loop
	glutMainLoop();

	return 0;
}