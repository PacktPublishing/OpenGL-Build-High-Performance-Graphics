#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

#include "..\\..\\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//screen resolution
const int WIDTH  = 512;
const int HEIGHT = 512;

//shader for rendering of image and convolution
GLSLShader shader;
GLSLShader convolution_shader;
GLSLShader* current_shader;

//vertex array and vertex buffer object IDs
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//texture image ID
GLuint textureID;

//vertices and indices arrays for fullscreen quad
glm::vec2 vertices[4];
GLushort indices[6];

//texture image filename
const string filename = "media/Lenna.png";

void OnInit() {
	GL_CHECK_ERRORS
	//load shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddUniform("textureMap");
		//pass values of constant uniforms at initialization
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	current_shader = &shader;

	GL_CHECK_ERRORS

	//load shader
	convolution_shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	convolution_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader_convolution.frag");
	//compile and link shader
	convolution_shader.CreateAndLinkProgram();
	convolution_shader.Use();
		//add attributes and uniforms
		convolution_shader.AddAttribute("vVertex");
		convolution_shader.AddUniform("textureMap");
		//pass values of constant uniforms at initialization
		glUniform1i(convolution_shader("textureMap"), 0);
	convolution_shader.UnUse();

	GL_CHECK_ERRORS

	//setup quad geometry
	//setup quad vertices
	vertices[0] = glm::vec2(0.0,0.0);
	vertices[1] = glm::vec2(1.0,0.0);
	vertices[2] = glm::vec2(1.0,1.0);
	vertices[3] = glm::vec2(0.0,1.0);

	//fill quad indices array
	GLushort* id=&indices[0];
	*id++ =0;
	*id++ =1;
	*id++ =2;
	*id++ =0;
	*id++ =2;
	*id++ =3;

	GL_CHECK_ERRORS

	//setup quad vao and vbo stuff
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//pass quad vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 2, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//pass quad indices to element array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS


	//load the image using SOIL
	int texture_width = 0, texture_height = 0, channels=0;
	GLubyte* pData = SOIL_load_image(filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);

	//vertically flip the image on Y axis since it is inverted
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

	//setup OpenGL texture and bind to texture unit 0
	glGenTextures(1, &textureID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		//set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		//allocate texture 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

	//free SOIL image data
	SOIL_free_image_data(pData);

	GL_CHECK_ERRORS

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {

	current_shader = 0;

	//Destroy shader
	shader.DeleteShaderProgram();
	convolution_shader.DeleteShaderProgram();


	//Destroy vao and vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	//Delete textures
	glDeleteTextures(1, &textureID);
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

//display function
void OnRender() {
	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//bind current shader
	current_shader->Use();
		//draw fullscreen quad
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	//unbind shader
	current_shader->UnUse();

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//keyboard event handler to change the output to convolved or normal image
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ' ': {
			if(current_shader == &shader) {
				current_shader = &convolution_shader;
				glutSetWindowTitle("Filtered image");
			} else {
				current_shader = &shader;
				glutSetWindowTitle("Normal image");
			}
		}break;
	}
	//call display function
 	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Convolution - OpenGL 3.3");
	
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

	cout<<"Press ' ' key to filter/unfilter\n";
	GL_CHECK_ERRORS

	//initialization of OpenGL
	OnInit();

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutKeyboardFunc(OnKey);

	//main loop call
	glutMainLoop();

	return 0;
}