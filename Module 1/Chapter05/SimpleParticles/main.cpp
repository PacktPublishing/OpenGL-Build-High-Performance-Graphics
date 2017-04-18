#define _USE_MATH_DEFINES
#include <cmath>
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

//output screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;

//particle shader, textured shader and a pointer to current shader
GLSLShader shader, texturedShader, *pCurrentShader;

//IDs for vertex array and buffer object
GLuint vaoID;
GLuint vboVerticesID; 

//total number of particles
const int MAX_PARTICLES = 10000;
 
//projection modelview and emitter transform matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);
glm::mat4 emitterXForm = glm::mat4(1);

//camera transformation variables
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, dist = -10;
float time = 0;
 
//particle texture filename 
const std::string texture_filename = "../media/particle.dds";

//texture OpenGL texture ID
GLuint textureID;

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

//OpenGL initialization function
void OnInit() {
	GL_CHECK_ERRORS
	//loader shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();	  
		//add attribute and uniform
		shader.AddUniform("MVP");
		shader.AddUniform("time"); 
	shader.UnUse();

	GL_CHECK_ERRORS

	//load textured rendering shader
	texturedShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	texturedShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/textured.frag");
	//compile and link shader
	texturedShader.CreateAndLinkProgram();
	texturedShader.Use();	  
		//add attribute and uniform
		texturedShader.AddUniform("MVP");
		texturedShader.AddUniform("time"); 
		texturedShader.AddUniform("textureMap");
		//set values of constant uniforms as initialization	
		glUniform1i(texturedShader("textureMap"),0);
	texturedShader.UnUse();
	 
	GL_CHECK_ERRORS

	//setup the vertex array object and vertex buffer object for the mesh
	//geometry handling 
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	//These calls marked start ATI/end ATI below are not required but on 
	//ATI cards i tested, i had to pass an allocated buffer object
	//uncomment the code marked with start/end ATI if running on an ATI hardware
	/// start ATI ///
	/*
	glGenBuffers(1, &vboVerticesID);  
	glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
	glBufferData (GL_ARRAY_BUFFER, sizeof(GLubyte)*MAX_PARTICLES, 0, GL_STATIC_DRAW);	 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,1,GL_UNSIGNED_BYTE, GL_FALSE, 0,0);
	*/
	/// end ATI ////

	GL_CHECK_ERRORS 
	
	//set particle size
	glPointSize(10);

	//enable blending and over blending operator
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
	//disable depth test
	glDisable(GL_DEPTH_TEST);
	 
	//setup emitter xform
	emitterXForm = glm::translate(glm::mat4(1), glm::vec3(0,0,0));
	emitterXForm = glm::rotate(emitterXForm, 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));  

	//load the texture
	int texture_width=0, texture_height=0, channels=0;
	GLubyte* pData = SOIL_load_image(texture_filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);
	if(pData == NULL) {
		cerr<<"Cannot load image: "<<texture_filename.c_str()<<endl;
		exit(EXIT_FAILURE);
	} 

	//Flip the image on Y axis
	int i,j;
	for( j = 0; j*2 < texture_height; ++j )
	{
		int index1 = j * texture_width * channels;
		int index2 = (texture_height - 1 - j) * texture_width * channels;
		for( i = texture_width * channels; i > 0; --i )
		{
			GLubyte temp = pData[index1];
			pData[index1] = pData[index2];
			pData[index2] = temp;
			++index1;
			++index2;
		}
	} 
	//get the image format
	GLenum format = GL_RGBA;
	switch(channels) {
		case 2:	format = GL_RG32UI; break;
		case 3: format = GL_RGB;	break;
		case 4: format = GL_RGBA;	break;
	}

	//generate OpenGL texture
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//allocate texture 
	glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);
	
	//release SOIL image data
	SOIL_free_image_data(pData);

	//set the particle shader as the current shader
	pCurrentShader = &shader;

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	//delete the particle texture
	glDeleteTextures(1, &textureID);

	//Destroy shader
	pCurrentShader = NULL;
	shader.DeleteShaderProgram();
	texturedShader.DeleteShaderProgram();

	//Destroy vao and vbo
	/// start ATI ///
	//glDeleteBuffers(1, &vboVerticesID); 
	/// end ATI ///

	glDeleteVertexArrays(1, &vaoID);

	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(60.0f, (float)w/h,0.1f,100.0f);
}

//display callback function
void OnRender() { 
	//get current time
	GLfloat time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	
	//clear colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transformation
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f)); 
    glm::mat4 MVP	= P*MV;
	 
	//bind the current shader
	pCurrentShader->Use();				
		//pass shader uniforms
		glUniform1f((*pCurrentShader)("time"), time);
		glUniformMatrix4fv((*pCurrentShader)("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*emitterXForm));
		//render points
			glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
	//unbind shader
	pCurrentShader->UnUse();
	
	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//call display function when idle
void OnIdle() {
	glutPostRedisplay();
}

//keyboard event handler to toggle use of textured and coloured particle system
void OnKey(unsigned char key, int x, int y) {
	switch (key) {
		case ' ': 
			if(pCurrentShader == &shader) {
				pCurrentShader = &texturedShader;  
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			} else {
				pCurrentShader = &shader; 
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		break;
	}
}

int main(int argc, char** argv) {
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);	
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Simple particles - OpenGL 3.3");

	//initialize glew
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

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize); 
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutIdleFunc(OnIdle);
	glutKeyboardFunc(OnKey);

	//main loop call
	glutMainLoop();	

	return 0;
}