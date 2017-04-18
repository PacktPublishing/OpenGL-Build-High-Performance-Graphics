#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shader reference
GLSLShader shader;

//vertex array and vertex buffer object IDs
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//out vertex struct for interleaved attributes
struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
};

//triangle vertices and indices
Vertex vertices[3];
GLushort indices[3];

//projection and modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//OpenGL initialization
void OnInit() {
	GL_CHECK_ERRORS
	//load the shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vColor");
		shader.AddUniform("MVP");
	shader.UnUse();

	GL_CHECK_ERRORS

	//setup triangle geometry
	//setup triangle vertices
	vertices[0].color=glm::vec3(1,0,0);
	vertices[1].color=glm::vec3(0,1,0);
	vertices[2].color=glm::vec3(0,0,1);

	vertices[0].position=glm::vec3(-1,-1,0);
	vertices[1].position=glm::vec3(0,1,0);
	vertices[2].position=glm::vec3(1,-1,0);

	//setup triangle indices
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	GL_CHECK_ERRORS

	//setup triangle vao and vbo stuff
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);
	GLsizei stride = sizeof(Vertex);

	glBindVertexArray(vaoID);

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//pass triangle verteices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,stride,0);
		GL_CHECK_ERRORS
		//enable vertex attribute array for colour
		glEnableVertexAttribArray(shader["vColor"]);
		glVertexAttribPointer(shader["vColor"], 3, GL_FLOAT, GL_FALSE,stride, (const GLvoid*)offsetof(Vertex, color));
		GL_CHECK_ERRORS
		//pass indices to element array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS 

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
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
	P = glm::ortho(-1,1,-1,1);
}

//display callback function
void OnRender() {

	//clear the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//bind the shader
	shader.Use();
		//pass the shader uniform
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV));
			//drwa triangle
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
	//unbind the shader
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
	glutCreateWindow("Simple triangle - OpenGL 3.3");

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

	//main loop call
	glutMainLoop();

	return 0;
}