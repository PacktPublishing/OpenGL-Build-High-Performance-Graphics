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

//screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;

//camera transform variables
int state = 0, oldX=0, oldY=0;
float rX=4, rY=50, dist = -2;

//grid object
#include "..\src\Grid.h"
CGrid* grid;

//modelview projection matrices
glm::mat4 MV,P;

float last_time=0, current_time=0;

//cube vertex array and vertex buffer object IDs
GLuint cubeVBOID;
GLuint cubeVAOID;
GLuint cubeIndicesID;

//pseudo iso-surface ray casting shader
GLSLShader shader;

//background colour
glm::vec4 bg=glm::vec4(0.5,0.5,1,1);

//volume dataset filename 
const std::string volume_file = "../media/Engine256.raw";

//volume dimensions
const int XDIM = 256;
const int YDIM = 256;
const int ZDIM = 256;

//volume texture ID
GLuint textureID;

//function that load a volume from the given raw data file and 
//generates an OpenGL 3D texture from it
bool LoadVolume() {
	std::ifstream infile(volume_file.c_str(), std::ios_base::binary);

	if(infile.good()) {
		//read the volume data file
		GLubyte* pData = new GLubyte[XDIM*YDIM*ZDIM];
		infile.read(reinterpret_cast<char*>(pData), XDIM*YDIM*ZDIM*sizeof(GLubyte));
		infile.close();

		//generate OpenGL texture
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);

		// set the texture parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//set the mipmap levels (base and max)
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);

		//allocate data with internal format and foramt as (GL_RED)			
		glTexImage3D(GL_TEXTURE_3D,0,GL_RED,XDIM,YDIM,ZDIM,0,GL_RED,GL_UNSIGNED_BYTE,pData);
		GL_CHECK_ERRORS

		//generate mipmaps
		glGenerateMipmap(GL_TEXTURE_3D);

		//delete the volume data allocated on heap
		delete [] pData;
		return true;
	} else {
		return false;
	}
}

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

//OpenGL initialization
void OnInit() {

	GL_CHECK_ERRORS

	//create a uniform grid of size 20x20 in XZ plane
	grid = new CGrid(20,20);

	GL_CHECK_ERRORS

	//Load the raycasting shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/raycaster.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/raycaster.frag");

	//compile and link the shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP");
		shader.AddUniform("volume");
		shader.AddUniform("camPos");
		shader.AddUniform("step_size");

		//pass constant uniforms at initialization
		glUniform3f(shader("step_size"), 1.0f/XDIM, 1.0f/YDIM, 1.0f/ZDIM);
		glUniform1i(shader("volume"),0);
	shader.UnUse();

	GL_CHECK_ERRORS

	//load volume data
	if(LoadVolume()) {
		std::cout<<"Volume data loaded successfully."<<std::endl; 
	} else {
		std::cout<<"Cannot load volume data."<<std::endl;
		exit(EXIT_FAILURE);
	}

	//set background colour
	glClearColor(bg.r, bg.g, bg.b, bg.a);
	
	//setup unit cube vertex array and vertex buffer objects
	glGenVertexArrays(1, &cubeVAOID);
	glGenBuffers(1, &cubeVBOID);
	glGenBuffers(1, &cubeIndicesID);

	//unit cube vertices 
	glm::vec3 vertices[8]={	glm::vec3(-0.5f,-0.5f,-0.5f),
							glm::vec3( 0.5f,-0.5f,-0.5f),
							glm::vec3( 0.5f, 0.5f,-0.5f),
							glm::vec3(-0.5f, 0.5f,-0.5f),
							glm::vec3(-0.5f,-0.5f, 0.5f),
							glm::vec3( 0.5f,-0.5f, 0.5f),
							glm::vec3( 0.5f, 0.5f, 0.5f),
							glm::vec3(-0.5f, 0.5f, 0.5f)};

	//unit cube indices
	GLushort cubeIndices[36]={0,5,4,
							  5,0,1,
							  3,7,6,
							  3,6,2,
							  7,4,6,
							  6,4,5,
							  2,1,3,
							  3,1,0,
							  3,0,7,
							  7,0,4,
							  6,5,2,
							  2,5,1};
	glBindVertexArray(cubeVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, cubeVBOID);
		//pass cube vertices to buffer object memory
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0].x), GL_STATIC_DRAW);

		GL_CHECK_ERRORS

		//enable vertex attributre array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,0,0);

		//pass indices to element array  buffer
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cubeIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	//enable depth test
	glEnable(GL_DEPTH_TEST);

	//set the over blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	shader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &cubeVAOID);
	glDeleteBuffers(1, &cubeVBOID);
	glDeleteBuffers(1, &cubeIndicesID);

	glDeleteTextures(1, &textureID);
	delete grid;
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//reset the viewport
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

	//get the camera position
	glm::vec3 camPos = glm::vec3(glm::inverse(MV)*glm::vec4(0,0,0,1));

	//clear colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//get the combined modelview projection matrix
    glm::mat4 MVP	= P*MV;
	
	//render grid
	grid->Render(glm::value_ptr(MVP));

	//enable blending and bind the cube vertex array object
	glEnable(GL_BLEND);
	glBindVertexArray(cubeVAOID);
		//bind the raycasting shader
		shader.Use();
			//pass shader uniforms
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniform3fv(shader("camPos"), 1, &(camPos.x));
				//render the cube
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		//unbind the raycasting shader
		shader.UnUse();
	//disable blending
	glDisable(GL_BLEND);

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Volume Rendering using GPU Ray Casting - OpenGL 3.3");

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

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);

	//main loop call
	glutMainLoop();

	return 0;
}