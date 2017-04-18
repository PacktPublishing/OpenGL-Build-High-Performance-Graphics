#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"
#include <fstream>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

//for floating point inaccuracy
const float EPSILON = 0.0001f;

#pragma comment(lib, "glew32.lib")

using namespace std;

//screen dimensions
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

//volume vertex array and buffer objects
GLuint volumeVBO;
GLuint volumeVAO;

//3D texture slicing shader
GLSLShader shader;

//maximum number of slices
const int MAX_SLICES = 512;

//sliced vertices
glm::vec3 vTextureSlices[MAX_SLICES*12];

//background colour
glm::vec4 bg=glm::vec4(0.5,0.5,1,1);

//volume data files
const std::string volume_file = "../media/Engine256.raw";

//dimensions of volume data
const int XDIM = 256;
const int YDIM = 256;
const int ZDIM = 256;

//total number of slices current used
int num_slices = 256;

//OpenGL volume texture id
GLuint textureID;

//flag to see if the view is rotated
//volume is resliced if the view is rotated
bool bViewRotated = false;

//unit cube vertices
glm::vec3 vertexList[8] = {glm::vec3(-0.5,-0.5,-0.5),
						   glm::vec3( 0.5,-0.5,-0.5),
						   glm::vec3(0.5, 0.5,-0.5),
						   glm::vec3(-0.5, 0.5,-0.5),
						   glm::vec3(-0.5,-0.5, 0.5),
						   glm::vec3(0.5,-0.5, 0.5),
						   glm::vec3( 0.5, 0.5, 0.5),
						   glm::vec3(-0.5, 0.5, 0.5)};

//unit cube edges
int edgeList[8][12] = {
	{ 0,1,5,6,   4,8,11,9,  3,7,2,10 }, // v0 is front
	{ 0,4,3,11,  1,2,6,7,   5,9,8,10 }, // v1 is front
	{ 1,5,0,8,   2,3,7,4,   6,10,9,11}, // v2 is front
	{ 7,11,10,8, 2,6,1,9,   3,0,4,5  }, // v3 is front
	{ 8,5,9,1,   11,10,7,6, 4,3,0,2  }, // v4 is front
	{ 9,6,10,2,  8,11,4,7,  5,0,1,3  }, // v5 is front
	{ 9,8,5,4,   6,1,2,0,   10,7,11,3}, // v6 is front
	{ 10,9,6,5,  7,2,3,1,   11,4,8,0 }  // v7 is front
};
const int edges[12][2]= {{0,1},{1,2},{2,3},{3,0},{0,4},{1,5},{2,6},{3,7},{4,5},{5,6},{6,7},{7,4}};

//current viewing direction
glm::vec3 viewDir;


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

	if(s == GLUT_UP)
		bViewRotated = false;
}

//mouse move event handler
void OnMouseMove(int x, int y)
{
	if (state == 0) {
		dist += (y - oldY)/50.0f;
	} else {
		rX += (y - oldY)/5.0f;
		rY += (x - oldX)/5.0f;
		bViewRotated = true;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//function to get the max (abs) dimension of the given vertex v
int FindAbsMax(glm::vec3 v) {
	v = glm::abs(v);
	int max_dim = 0;
	float val = v.x;
	if(v.y>val) {
		val = v.y;
		max_dim = 1;
	}
	if(v.z > val) {
		val = v.z;
		max_dim = 2;
	}
	return max_dim;
}

//main slicing function
void SliceVolume() {

	//get the max and min distance of each vertex of the unit cube
	//in the viewing direction
	float max_dist = glm::dot(viewDir, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for(int i=1;i<8;i++) {
		//get the distance between the current unit cube vertex and 
		//the view vector by dot product
		float dist = glm::dot(viewDir, vertexList[i]);

		//if distance is > max_dist, store the value and index
		if(dist > max_dist) {
			max_dist = dist;
			max_index = i;
		}

		//if distance is < min_dist, store the value 
		if(dist<min_dist)
			min_dist = dist;
	}
	//find tha abs maximum of the view direction vector
	int max_dim = FindAbsMax(viewDir);

	//expand it a little bit
	min_dist -= EPSILON;
	max_dist += EPSILON;

	//local variables to store the start, direction vectors, 
	//lambda intersection values
	glm::vec3 vecStart[12];
	glm::vec3 vecDir[12];
	float lambda[12];
	float lambda_inc[12];
	float denom = 0;

	//set the minimum distance as the plane_dist
	//subtract the max and min distances and divide by the 
	//total number of slices to get the plane increment
	float plane_dist = min_dist;
	float plane_dist_inc = (max_dist-min_dist)/float(num_slices);

	//for all edges
	for(int i=0;i<12;i++) {
		//get the start position vertex by table lookup
		vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

		//get the direction by table lookup
		vecDir[i] = vertexList[edges[edgeList[max_index][i]][1]]-vecStart[i];

		//do a dot of vecDir with the view direction vector
		denom = glm::dot(vecDir[i], viewDir);

		//determine the plane intersection parameter (lambda) and 
		//plane intersection parameter increment (lambda_inc)
		if (1.0 + denom != 1.0) {
			lambda_inc[i] =  plane_dist_inc/denom;
			lambda[i]     = (plane_dist - glm::dot(vecStart[i],viewDir))/denom;
		} else {
			lambda[i]     = -1.0;
			lambda_inc[i] =  0.0;
		}
	}

	//local variables to store the intesected points
	//note that for a plane and sub intersection, we can have 
	//a minimum of 3 and a maximum of 6 vertex polygon
	glm::vec3 intersection[6];
	float dL[12];

	//loop through all slices
	for(int i=num_slices-1;i>=0;i--) {
		
		//determine the lambda value for all edges
		for(int e = 0; e < 12; e++)
		{
			dL[e] = lambda[e] + i*lambda_inc[e];
		}

		//if the values are between 0-1, we have an intersection at the current edge
		//repeat the same for all 12 edges
		if  ((dL[0] >= 0.0) && (dL[0] < 1.0))	{
			intersection[0] = vecStart[0] + dL[0]*vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0))	{
			intersection[0] = vecStart[1] + dL[1]*vecDir[1];
		}
		else if ((dL[3] >= 0.0) && (dL[3] < 1.0))	{
			intersection[0] = vecStart[3] + dL[3]*vecDir[3];
		}
		else continue;

		if ((dL[2] >= 0.0) && (dL[2] < 1.0)){
			intersection[1] = vecStart[2] + dL[2]*vecDir[2];
		}
		else if ((dL[0] >= 0.0) && (dL[0] < 1.0)){
			intersection[1] = vecStart[0] + dL[0]*vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0)){
			intersection[1] = vecStart[1] + dL[1]*vecDir[1];
		} else {
			intersection[1] = vecStart[3] + dL[3]*vecDir[3];
		}

		if  ((dL[4] >= 0.0) && (dL[4] < 1.0)){
			intersection[2] = vecStart[4] + dL[4]*vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)){
			intersection[2] = vecStart[5] + dL[5]*vecDir[5];
		} else {
			intersection[2] = vecStart[7] + dL[7]*vecDir[7];
		}
		if	((dL[6] >= 0.0) && (dL[6] < 1.0)){
			intersection[3] = vecStart[6] + dL[6]*vecDir[6];
		}
		else if ((dL[4] >= 0.0) && (dL[4] < 1.0)){
			intersection[3] = vecStart[4] + dL[4]*vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)){
			intersection[3] = vecStart[5] + dL[5]*vecDir[5];
		} else {
			intersection[3] = vecStart[7] + dL[7]*vecDir[7];
		}
		if	((dL[8] >= 0.0) && (dL[8] < 1.0)){
			intersection[4] = vecStart[8] + dL[8]*vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)){
			intersection[4] = vecStart[9] + dL[9]*vecDir[9];
		} else {
			intersection[4] = vecStart[11] + dL[11]*vecDir[11];
		}

		if ((dL[10]>= 0.0) && (dL[10]< 1.0)){
			intersection[5] = vecStart[10] + dL[10]*vecDir[10];
		}
		else if ((dL[8] >= 0.0) && (dL[8] < 1.0)){
			intersection[5] = vecStart[8] + dL[8]*vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)){
			intersection[5] = vecStart[9] + dL[9]*vecDir[9];
		} else {
			intersection[5] = vecStart[11] + dL[11]*vecDir[11];
		}

		//after all 6 possible intersection vertices are obtained,
		//we calculated the proper polygon indices by using indices of a triangular fan
		int indices[]={0,1,2, 0,2,3, 0,3,4, 0,4,5};

		//Using the indices, pass the intersection vertices to the vTextureSlices vector
		for(int i=0;i<12;i++)
			vTextureSlices[count++]=intersection[indices[i]];
	}

	//update buffer object with the new vertices
	glBindBuffer(GL_ARRAY_BUFFER, volumeVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0,  sizeof(vTextureSlices), &(vTextureSlices[0].x));
}

//OpenGL initialization
void OnInit() {

	GL_CHECK_ERRORS

	//create a uniform grid of size 20x20 in XZ plane
	grid = new CGrid(20,20);

	GL_CHECK_ERRORS
		 
	//Load the texture slicing shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/textureSlicer.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/textureSlicer.frag");

	//compile and link the shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP");
		shader.AddUniform("volume");
		//pass constant uniforms at initialization
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

	//setup the current camera transform and get the view direction vector
	glm::mat4 T	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//get the current view direction vector
	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//setup the vertex array and buffer objects
	glGenVertexArrays(1, &volumeVAO);
	glGenBuffers(1, &volumeVBO);

	glBindVertexArray(volumeVAO);
	glBindBuffer (GL_ARRAY_BUFFER, volumeVBO);

	//pass the sliced vertices vector to buffer object memory
	glBufferData (GL_ARRAY_BUFFER, sizeof(vTextureSlices), 0, GL_DYNAMIC_DRAW);

	GL_CHECK_ERRORS
	
	//enable vertex attribute array for position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,0,0);

	glBindVertexArray(0);

	//slice the volume dataset initially
	SliceVolume();
	 
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	shader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &volumeVAO);
	glDeleteBuffers(1, &volumeVBO);

	glDeleteTextures(1, &textureID);
	delete grid;
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//setup the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}

//display function
void OnRender() {
	GL_CHECK_ERRORS
	//setup the camera transform
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//get the viewing direction
	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//get the combined modelview projection matrix
    glm::mat4 MVP	= P*MV;

	//render the grid object
	grid->Render(glm::value_ptr(MVP));

	//if view is rotated, reslice the volume
	if(bViewRotated)
	{
		SliceVolume();
	}

	//enable alpha blending (use over operator)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//bind volume vertex array object
	glBindVertexArray(volumeVAO);
		//use the volume shader
		shader.Use();
			//pass the shader uniform
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
				//draw the triangles
				glDrawArrays(GL_TRIANGLES, 0, sizeof(vTextureSlices)/sizeof(vTextureSlices[0]));
		//unbind the shader
		shader.UnUse();

	//disable blending
	glDisable(GL_BLEND);

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//keyboard function to change the number of slices
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case '-':
			num_slices--;
			break;

		case '+':
			num_slices++;
			break;
	}
	//check the range of num_slices variable
	num_slices = min(MAX_SLICES, max(num_slices,3));

	//slice the volume
	SliceVolume();

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
	glutCreateWindow("Volume Rendering using 3D Texture Slicing - OpenGL 3.3");
	
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
	glutKeyboardFunc(OnKey);

	//main loop call
	glutMainLoop();

	return 0;
}