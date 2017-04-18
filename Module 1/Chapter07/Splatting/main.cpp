#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "..\src\GLSLShader.h"
#include <fstream>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);


#pragma comment(lib, "glew32.lib")

using namespace std;

//screen dimensions
const int WIDTH  = 1280;
const int HEIGHT = 960;

//number of times to down sample
const int DOWN_SAMPLE = 2;
//down sampled image size
const int IMAGE_WIDTH = WIDTH/DOWN_SAMPLE;
const int IMAGE_HEIGHT = HEIGHT/DOWN_SAMPLE;

//camera transform variables
int state = 0, oldX=0, oldY=0;
float rX=4, rY=50, dist = -2;

//modelview and projection matrices
glm::mat4 MV,P;
 
//volume splatter vertex array and vertex buffer obejct IDs
GLuint volumeSplatterVBO;
GLuint volumeSplatterVAO;

//shaders for splatter, gaussian smoothing and fillscreen quad rendering
GLSLShader shader, gaussianV_shader, gaussianH_shader, quadShader;

//background colour
glm::vec4 bg=glm::vec4(0,0,0,1);

//volume data filename
const std::string volume_file = "../media/Engine256.raw";
 
//VolumeSplatter instance
#include "VolumeSplatter.h"
VolumeSplatter* splatter;

//filter FBO and normal FBO and renderbuffer IDs
GLuint filterFBOID, fboID, rboID;

//blur texture IDs
GLuint blurTexID[2];
//colour attachment texture ID
GLuint texID;

//quad vertex array and vertex buffer object IDs
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

//mouse event handler
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

//OpenGL initialziation function
void OnInit() {

	GL_CHECK_ERRORS

	//create volume splatter instance
	splatter = new VolumeSplatter();
	//set volume dimentsions
	splatter->SetVolumeDimensions(256,256,256);
	//load volume data
	splatter->LoadVolume(volume_file);
	//set the required isosurface value
	splatter->SetIsosurfaceValue(40);
	//set the number of sampling voxels
	splatter->SetNumSamplingVoxels(64,64,64);
	std::cout<<"Generating point splats ...";
	//splat volumes
	splatter->SplatVolume();
	std::cout<<"Done."<<std::endl;

	//generate the vertex array and vertex buffer objects
	glGenVertexArrays(1, &volumeSplatterVAO);
	glGenBuffers(1, &volumeSplatterVBO);
	glBindVertexArray(volumeSplatterVAO);
	glBindBuffer (GL_ARRAY_BUFFER, volumeSplatterVBO);

	//pass the vertices from the splatter to vertex buffer object
	glBufferData (GL_ARRAY_BUFFER, splatter->GetTotalVertices()*sizeof(Vertex), splatter->GetVertexPointer(), GL_STATIC_DRAW);

	//enable vertex attrib array for positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);

	//enable vertex attrib array for normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),(const GLvoid*)offsetof(Vertex, normal));

	GL_CHECK_ERRORS

	//load shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/splatShader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/splatShader.frag");
	//compile and link shader 
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MV");
		shader.AddUniform("N");
		shader.AddUniform("P");
		shader.AddUniform("splatSize");
		//set constant uniforms once
		glUniform1f(shader("splatSize"), 256/64);
	shader.UnUse();

	GL_CHECK_ERRORS

	//load the horizontal Gaussian smoothing shader
	gaussianH_shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Passthrough.vert");
	gaussianH_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/GaussH.frag");

	//compile and link shader
	gaussianH_shader.CreateAndLinkProgram();
	gaussianH_shader.Use();

		//add shader attributes and uniforms
		gaussianH_shader.AddAttribute("vVertex");
		gaussianH_shader.AddUniform("textureMap");
		//pass constant uniforms once
		//this shader reads texture attached to texture unit 1
		glUniform1i(gaussianH_shader("textureMap"),1);
	gaussianH_shader.UnUse();

	GL_CHECK_ERRORS

	//load the vertical Gaussian smoothing shader
	gaussianV_shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Passthrough.vert");
	gaussianV_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/GaussV.frag");

	//compile and link shader
	gaussianV_shader.CreateAndLinkProgram();
	gaussianV_shader.Use();
		//add attribute and uniforms 
		gaussianV_shader.AddAttribute("vVertex");
		gaussianV_shader.AddUniform("textureMap");
		//pass constant uniforms once
		//this shader reads texture attached to texture unit 0
		glUniform1i(gaussianV_shader("textureMap"),0);
	gaussianV_shader.UnUse();

	GL_CHECK_ERRORS

	//load quad shader
	quadShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/quad_shader.vert");
	quadShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/quad_shader.frag");

	//compile and link shader 
	quadShader.CreateAndLinkProgram();
	quadShader.Use();
		//add attributes and uniforms
		quadShader.AddAttribute("vVertex");
		quadShader.AddUniform("MVP");
		quadShader.AddUniform("textureMap");
		//pass constant uniforms once
		//this shader reads texture attached to texture unit 2
		glUniform1i(quadShader("textureMap"), 2);
	quadShader.UnUse();

	GL_CHECK_ERRORS

	//set the background colour
	glClearColor(bg.r, bg.g, bg.b, bg.a);

	//enable depth test and culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//enable vertex shader to change the point size by 
	//writing to gl_PointSize register
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	//setup filtering fbo
	glGenFramebuffers(1,&filterFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);

	//generate two blur textures and bind to texture unit 1 and 2
	//these are attached to colour attachment 0 and 1 respectively
	glGenTextures(2, blurTexID);
	for(int i=0;i<2;i++) {
		glActiveTexture(GL_TEXTURE1+i);
		glBindTexture(GL_TEXTURE_2D, blurTexID[i]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,IMAGE_WIDTH,IMAGE_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);		
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,blurTexID[i],0);
	}

	//check framebuffer completeness status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Filtering FBO setup successful."<<endl;
	} else {
		cout<<"Problem in Filtering FBO setup."<<endl;
	}

	GL_CHECK_ERRORS

	//setup the scene FBO with a render buffer object (RBO)
	glGenFramebuffers(1,&fboID);
	glGenRenderbuffers(1, &rboID);
	//generate the colour texture for attachment
	glGenTextures(1, &texID);
	//bind the FBO and RBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID);
	//bind the texture to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);

	GL_CHECK_ERRORS
	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	//allocate the texture object
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,IMAGE_WIDTH,IMAGE_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);

	GL_CHECK_ERRORS

	//bind texture as FBO attachment at colour attachment 0
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, texID, 0);
	//bind the render buffer as depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, IMAGE_WIDTH, IMAGE_HEIGHT);

	//check for FBO completeness
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Offscreen rendering FBO setup successful."<<endl;
	} else {
		cout<<"Problem in offscreen rendering FBO setup."<<endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_CHECK_ERRORS

	//setup the quad vertices
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//setup quad indices
	GLushort indices[6]={0,1,2,0,2,3};

	//setup quad vertex array and vertex buffer objects
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	GL_CHECK_ERRORS

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//pass quad vertices to vertex buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS
		//enable vertex attrib array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);

		//pass quad indices to element array buffer
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	GL_CHECK_ERRORS

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	glDeleteFramebuffers(1, &filterFBOID);
	glDeleteTextures(2, blurTexID);

	glDeleteFramebuffers(1,&fboID);
	glDeleteTextures(1, &texID);
	glDeleteRenderbuffers(1, &rboID);

	glDeleteVertexArrays(1, &volumeSplatterVAO);
	glDeleteBuffers(1, &volumeSplatterVBO);

	delete splatter;

	quadShader.DeleteShaderProgram();
	gaussianH_shader.DeleteShaderProgram();
	gaussianV_shader.DeleteShaderProgram();
	shader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

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

	//bind FBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);

	//set the viewport to the size of FBO colour attachment
	glViewport(0,0, IMAGE_WIDTH, IMAGE_HEIGHT);
	//set colour attachment 0 as draw buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//set the modelling transform to bring the splatting output to origin
	glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(-0.5,-0.5,-0.5));

	//bind the splatter vertex array object
	glBindVertexArray(volumeSplatterVAO);
		//bind the splatting shader
		shader.Use();
			//set the shader uniforms
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV*T));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV*T))));
			glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));
				//draw points
				glDrawArrays(GL_POINTS, 0, splatter->GetTotalVertices());
		//unbind the splatting shader
		shader.UnUse();

	//bind the quad vertex array object
	glBindVertexArray(quadVAOID);

	//bind the filtering FBO
	glBindFramebuffer(GL_FRAMEBUFFER, filterFBOID);
	//set colour attachment 0 as draw buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//set vertical Gaussian smoothing shader
		gaussianV_shader.Use();
			//draw fullscreen quad
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	//set colour attachment 1 as draw buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
		//set horizontal Gaussian smoothing result 
		gaussianH_shader.Use();
			//draw fullscreen quad
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	//unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//reset the default back buffer
	glDrawBuffer(GL_BACK_LEFT);
	//reset the default viewport
	glViewport(0,0,WIDTH, HEIGHT);

	//set quad shader
	quadShader.Use();
		//draw fullscreen quad to display the final filtered result
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	//remove quad shader
	quadShader.UnUse();

	//unbind vertex array object
	glBindVertexArray(0);

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