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

//3D texture slicing shader, shadowShader, flatShader and quadShader
GLSLShader shader, shaderShadow, flatShader, quadShader;

//maximum number of slices
const int MAX_SLICES = 512;

//sliced vertices
glm::vec3 vTextureSlices[MAX_SLICES*12];
 
//volume data files
const std::string volume_file = "../media/Engine256.raw";

//dimensions of volume data
const int XDIM = 256;
const int YDIM = 256;
const int ZDIM = 256;

//total number of slices current used
int num_slices =  256;

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

//light vertex array and buffer object IDs
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosOS=glm::vec3(0, 2,0); //objectspace light position

//light spherical coordinates
float theta = 0.66f;
float phi = -1.0f;
float radius = 2;

//current viewing direction vector, light vector and half angle vector
glm::vec3	viewVec, 
			lightVec, 
			halfVec;

//offscreen texture for light and eye buffer
GLuint lightBufferID, eyeBufferID;

GLuint colorTexID; //FBO attachment ID

//light blending FBO ID
GLuint lightFBOID;

//quad vertex array and buffer object ID
GLuint quadVAOID, quadVBOID;


//total number of down samples
const int DOWN_SAMPLE = 2;

//downsampled image width and height
const int IMAGE_WIDTH = WIDTH/DOWN_SAMPLE;
const int IMAGE_HEIGHT = HEIGHT/DOWN_SAMPLE;

glm::mat4 MV_L; //light modelview matrix
glm::mat4 P_L;	//light projection matrix
glm::mat4 B;    //light bias matrix
glm::mat4 BP;   //light bias and projection matrix combined
glm::mat4 S;    //light's combined BPMV matrix

//flag to indicate if the view is inverted
bool bIsViewInverted = false;
//colour of light
glm::vec4 lightColor=glm::vec4(1.0,1.0,1.0,1.0);
//colout of light attenuation 
glm::vec3 lightAttenuation=glm::vec3(0.1,0.2,0.3);

//shadow alpha value
float fShadowAlpha = 1;
 
//attachment IDs for FBO colour attachments
GLenum attachIDs[2]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

//function to create an OpenGL texture of specified width (w) and height (h) with the given internal 
//format (internalFormat) and format (format).
//The function returns the texture ID of the newly created texture.
GLuint CreateTexture(const int w,const int h, GLenum internalFormat, GLenum format) {
	GLuint texid;
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_FLOAT, 0);
    return texid;
}

//initialization of FBO
void InitFBO() {
	
	//Generate an FBO for rendering to texture
	glGenFramebuffers(1, &lightFBOID);

	//setup two textures for storing the light and eye buffer accumulated result
	glGenTextures (1, &lightBufferID);
	glGenTextures (1, &eyeBufferID);

	GL_CHECK_ERRORS

	//create light and eye buffer textures
	glActiveTexture(GL_TEXTURE2);
	lightBufferID = CreateTexture(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA16F, GL_RGBA);
	eyeBufferID = CreateTexture(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA16F, GL_RGBA);

	//bind the FBO 
	glBindFramebuffer(GL_FRAMEBUFFER, lightFBOID);
	//set the colour attachments
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightBufferID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, eyeBufferID, 0);

	//check framebuffer completeness status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE )
		printf("Light FBO setup successful !!! \n");
	else
		printf("Problem with Light FBO setup");

	//unbind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//release all FBO related resources
void ShutdownFBO() {
	glDeleteFramebuffers(1, &lightFBOID);
	glDeleteTextures (1, &lightBufferID);
	glDeleteTextures (1, &eyeBufferID);
}

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
		glActiveTexture(GL_TEXTURE0);
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
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
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
	} else if(state ==2) {
		theta += (oldX - x)/60.0f;
		phi += (y - oldY)/60.0f;

		//modify the light source position
		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

		//update light's MV matrix
		MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0), glm::vec3(0,1,0));
		S = BP*MV_L;
	} else {
		rX += (y - oldY)/5.0f;
		rY += (x - oldX)/5.0f;
		bViewRotated = true;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//mouse wheel scroll event handler to update the light radius
void OnMouseWheel(int button, int dir, int x, int y) {

	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }

	radius = max(radius,0.0f);

	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//update light's MV matrix
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	S = BP*MV_L;
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
	float max_dist = glm::dot(halfVec, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for(int i=1;i<8;i++) {
		//get the distance between the current unit cube vertex and 
		//the view vector by dot product
		float dist = glm::dot(halfVec, vertexList[i]);

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
	int max_dim = FindAbsMax(halfVec);

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

		//do a dot of vecDir with the half way vector
		denom = glm::dot(vecDir[i], halfVec);

		//determine the plane intersection parameter (lambda) and 
		//plane intersection parameter increment (lambda_inc)
		if (1.0 + denom != 1.0) {
			lambda_inc[i] =  plane_dist_inc/denom;
			lambda[i]     = (plane_dist - glm::dot(vecStart[i],halfVec))/denom;
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
		
	//load the texture slicing shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/textureSlicer.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/textureSlicer.frag");

	//compile and link the shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP");
		shader.AddUniform("color");
		shader.AddUniform("volume");

		//pass constant uniforms at initialization
		glUniform1i(shader("volume"),0);
		glUniform4f(shader("color"),lightAttenuation.x*fShadowAlpha, lightAttenuation.y * fShadowAlpha, lightAttenuation.z * fShadowAlpha, 1);

	shader.UnUse();

	GL_CHECK_ERRORS

	//load the slicing shadow shader
	shaderShadow.LoadFromFile(GL_VERTEX_SHADER, "shaders/slicerShadow.vert");
	shaderShadow.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/slicerShadow.frag");

	//compile and link the shader
	shaderShadow.CreateAndLinkProgram();
	shaderShadow.Use();
		//add attributes and uniforms
		shaderShadow.AddAttribute("vVertex");
		shaderShadow.AddUniform("MVP");
		shaderShadow.AddUniform("S");
		shaderShadow.AddUniform("color");
		shaderShadow.AddUniform("shadowTex");
		shaderShadow.AddUniform("volume");

		//pass constant uniforms at initialization
		glUniform1i(shaderShadow("volume"),0);
		glUniform1i(shaderShadow("shadowTex"),1);
		glUniform4f(shaderShadow("color"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);

	shaderShadow.UnUse();

	//load the flat shader
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");

	//compile and link the shader
	flatShader.CreateAndLinkProgram();
	flatShader.Use();
		//add shader attributes and uniforms
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP");
	flatShader.UnUse();

	GL_CHECK_ERRORS

	//load the quad shader
	quadShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/quad_shader.vert");
	quadShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/quad_shader.frag");

	//compile and link the shader
	quadShader.CreateAndLinkProgram();
	quadShader.Use();
		//add attribute and uniforms
		quadShader.AddAttribute("vVertex");
		quadShader.AddUniform("MVP");
		quadShader.AddUniform("textureMap");
		//pass constant uniforms at initialization
		glUniform1i(quadShader("textureMap"),1);
	quadShader.UnUse();

	GL_CHECK_ERRORS

	//load volume data
	if(LoadVolume()) {
		std::cout<<"Volume data loaded successfully."<<std::endl;
		 
	} else {
		std::cout<<"Cannot load volume data."<<std::endl;
		exit(EXIT_FAILURE);
	}

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

	//setup vao and vbo stuff for the light position crosshair
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//setup the vertex array object and vertex buffer object
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);

	glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);

	//pass the crosshair geomtry to the buffer object
	glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
	GL_CHECK_ERRORS

	//enable vertex attribute array
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);

	GL_CHECK_ERRORS

	//setup the fullscreen quad vertices
	glm::vec2 quadVerts[6];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,0);
	quadVerts[4] = glm::vec2(1,1);
	quadVerts[5] = glm::vec2(0,1);

	//setup the quad vertex array and vertex buffer objects
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);

	glBindVertexArray(quadVAOID);
	glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
	
	//pass the quad vertices array to buffer object memory
	glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//enable vertex attribute array
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);

	//get the light object space position
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//setup the current camera transform and get the view direction vector
	glm::mat4 T	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//get the current view direction vector
	viewVec = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//slice the volume dataset initially
	SliceVolume();

	//initialize FBO and associate colour attachments
	InitFBO();

	//set the light MV, P and bias matrices
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	P_L  = glm::perspective(45.0f,1.0f,1.0f, 200.0f);
	B    = glm::scale(glm::translate(glm::mat4(1),glm::vec3(0.5,0.5,0.5)), glm::vec3(0.5,0.5,0.5));
	BP   = B*P_L;
	S    = BP*MV_L;

	cout<<"Initialization successfull"<<endl;

	//set texture unit 1 as the active texture unit
	glActiveTexture(GL_TEXTURE1);
}

//release all allocated resources
void OnShutdown() {
	ShutdownFBO();

	shader.DeleteShaderProgram();
	shaderShadow.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();
	quadShader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &volumeVAO);
	glDeleteBuffers(1, &volumeVBO);

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);

	glDeleteTextures(1, &textureID);
	delete grid;
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}
//function to render slice from the point of view of eye in the eye buffer
void DrawSliceFromEyePointOfView(const int i) {
	GL_CHECK_ERRORS

	//set the colour attachment 1 (eyebuffer) as the draw buffer
	glDrawBuffer(attachIDs[1]);

	//reset the current viewport to the colour attachment size
	glViewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);

	GL_CHECK_ERRORS

	//check if the view direction is inverted
	if(bIsViewInverted) {
		//if yes, we swap the blending equation
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
	} else {
		//otherwise we use the conventiona blending equation
		//we use GL_ONE as the source factor because we want 
		//to keep the source color
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	//draw the slice 
	glDrawArrays(GL_TRIANGLES, 12*i, 12);

	GL_CHECK_ERRORS

}
//function to render slice from the point of view of light in the light buffer
void DrawSliceFromLightPointOfView(const int i) {

	//set the color attachment0 (lightbuffer) as the draw buffer
	glDrawBuffer(attachIDs[0]);

	//set the viewport to the size of the colour attachment
	glViewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);

	//we use conventional "over" compositing for light buffer
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//draw the slice 
	glDrawArrays(GL_TRIANGLES, 12*i, 12);
}

//function to render the slices into the light or the eye buffer
void DrawSlices(glm::mat4 MVP) {

	GL_CHECK_ERRORS

	//clear light buffer
	glBindFramebuffer(GL_FRAMEBUFFER, lightFBOID);
	glDrawBuffer(attachIDs[0]);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT );

	//clear eye buffer
	glDrawBuffer(attachIDs[1]);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT  );

	GL_CHECK_ERRORS

	//bind the volume vertex array object
	glBindVertexArray(volumeVAO);

	//for all slices
	for(int i =0;i<num_slices;i++) {
		//bind the shadow shader
		shaderShadow.Use();
		//set the shadow shader uniforms
		glUniformMatrix4fv(shaderShadow("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(shaderShadow("S"), 1, GL_FALSE, glm::value_ptr(S));

		//bind the light buffer as the current texture
		glBindTexture(GL_TEXTURE_2D, lightBufferID);

		//draw slice from the point of view of eye into eye buffer
		DrawSliceFromEyePointOfView(i);

		GL_CHECK_ERRORS

		//use the normal volume shader 
		shader.Use();

		//set the shader uniform
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(P_L*MV_L));

		//render slice form the point of view of light in the light buffer
		DrawSliceFromLightPointOfView(i);

		GL_CHECK_ERRORS
	}
	//unbind vertex array object 
	glBindVertexArray(0);

	//unbind the FBO and switch to draw to the back buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK_LEFT);

	GL_CHECK_ERRORS

	//reset the viewport to draw to the entire screen
	glViewport(0,0,WIDTH, HEIGHT);

	//set the eye buffer as the current texture
	glBindTexture(GL_TEXTURE_2D, eyeBufferID);

	//bind the fullscreen quad vertex array object
	glBindVertexArray(quadVAOID);

	//use the quad shader
	quadShader.Use();
		//draw the full screen quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
	quadShader.UnUse();

	glBindVertexArray(0);

	GL_CHECK_ERRORS
}

//display callback function
void OnRender() {
	GL_CHECK_ERRORS

	//set the camera transform
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	 
	//get the view vector
	viewVec = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//get the light vector
	lightVec = glm::normalize(lightPosOS);

	//check if the view is inverted
	bIsViewInverted = glm::dot(viewVec, lightVec)<0;

	//get the half way vector between the light and the view vector
	halfVec = glm::normalize( (bIsViewInverted?-viewVec:viewVec) + lightVec);

	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//get the combined modelview projection matrix
    glm::mat4 MVP	= P*MV;

	//render the grid
	grid->Render(glm::value_ptr(MVP));

	//slice the volume
	SliceVolume();

	//render the half angle sliced volume
	glEnable(GL_BLEND);
		DrawSlices(MVP);
	glDisable(GL_BLEND);

	//render the light crosshair gizmo
	glBindVertexArray(lightVAOID); {
		//set the light's transform
		glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
		//bind the flat shader
		flatShader.Use();
			//set the shader uniform
			glUniformMatrix4fv(flatShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				//render 6 lines
				glDrawArrays(GL_LINES, 0, 6);
		//unbind the flat shader
		flatShader.UnUse();
	}
	
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
	glutCreateWindow("Volume lighting using Half Angle Slicing - OpenGL 3.3");

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
	glutMouseWheelFunc(OnMouseWheel);
	glutKeyboardFunc(OnKey);

	//main loop call
	glutMainLoop();

	return 0;
}