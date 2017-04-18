#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

#include "GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shader
GLSLShader shader;

//vertex array and vertex buffer object ids
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//mesh vertices and indices
glm::vec3 vertices[4];
GLushort indices[6];

//projection and modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -35;

//number of sub-divisions
int sub_divisions = 1;

//modelling matrix of each instance
glm::mat4 M[4];

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

//key event handler to increase/decrease number of sub-divisions
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ',':	sub_divisions--; break;
		case '.':	sub_divisions++; break;
	}

	sub_divisions = max(1,min(8, sub_divisions));

	//call the display function 
	glutPostRedisplay();
}

//OpenGL initialization
void OnInit() {

	//set the instance modeling matrix
	M[0] = glm::translate(glm::mat4(1), glm::vec3(-5,0,-5));
	M[1] = glm::translate(M[0], glm::vec3(10,0,0));
	M[2] = glm::translate(M[1], glm::vec3(0,0,10));
	M[3] = glm::translate(M[2], glm::vec3(-10,0,0));

	GL_CHECK_ERRORS

	//load the shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_GEOMETRY_SHADER, "shaders/shader.geom");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//create and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attribute and uniform
		shader.AddAttribute("vVertex");
		shader.AddUniform("PV");
		shader.AddUniform("M");
		shader.AddUniform("sub_divisions");

		//set values of constant uniforms at initialization
		glUniform1i(shader("sub_divisions"), sub_divisions);
		glUniformMatrix4fv(shader("M"), 4, GL_FALSE, glm::value_ptr(M[0]));

	shader.UnUse();

	GL_CHECK_ERRORS

	//setup quad geometry
	//setup quad vertices
	vertices[0] = glm::vec3(-5,0,-5);
	vertices[1] = glm::vec3(-5,0,5);
	vertices[2] = glm::vec3(5,0,5);
	vertices[3] = glm::vec3(5,0,-5);

	//setup quad indices
	GLushort* id=&indices[0];
 	*id++ = 0;
	*id++ = 1;
	*id++ = 2;

	*id++ = 0;
	*id++ = 2;
	*id++ = 3;

	GL_CHECK_ERRORS

	//setup quad vao and vbo stuff
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//pass the quad vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//pass the quad indices to element array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS

	//set the polygon mode to render lines
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GL_CHECK_ERRORS
		
	cout<<"Initialization successfull"<<endl;
}

//delete all allocated resources
void OnShutdown() {
	//Destroy shader
	shader.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport size
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	//setup the projection matrix
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.01f, 10000.f);
}

//display callback function
void OnRender() {
	//clear colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transformation 
	glm::mat4 T	 = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 V	 = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 PV = P*V;

	//bind the shader
	shader.Use();
		//set the shader uniforms
		glUniformMatrix4fv(shader("PV"), 1, GL_FALSE, glm::value_ptr(PV));
		glUniform1i(shader("sub_divisions"), sub_divisions);
			//render instanced geometry
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, 4);
		//unbind shader
		shader.UnUse();

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
	glutCreateWindow("Simple plane subdivision using geometry shader instanced - OpenGL 3.3");

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
	glutKeyboardFunc(OnKey);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);

	//call main loop
	glutMainLoop();

	return 0;
}