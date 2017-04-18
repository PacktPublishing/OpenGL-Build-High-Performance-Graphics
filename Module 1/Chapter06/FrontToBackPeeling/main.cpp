#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//set screen dimensions
const int WIDTH  = 1280;
const int HEIGHT = 960;

//camera transform variables
int state = 0, oldX=0, oldY=0;
float rX=0, rY=300, dist = -10;

//grid object
#include "..\src\Grid.h"
CGrid* grid;

//modelview projection and rotation matrices
glm::mat4 MV,P,R;

//constants for box colours 
glm::vec4 box_colors[3]={glm::vec4(1,0,0,0.5),
						 glm::vec4(0,1,0,0.5),
						 glm::vec4(0,0,1,0.5)
							};
 
//auto rotate angle
float angle = 0;

//FBO id
GLuint fbo[2];
//FBO colour attachment IDs
GLuint texID[2];
//FBO depth attachment IDs
GLuint depthTexID[2];

//colour blending FBO ID
GLuint colorBlenderFBOID;
//colour blend FBO colour attachment texture ID
GLuint colorBlenderTexID;

//occlusion query ID
GLuint queryId;

//fullscreen quad vao and vbos
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

//cube vertex array and vertex buffer object IDs
GLuint cubeVBOID;
GLuint cubeVAOID;
GLuint cubeIndicesID;

//shaders for cube, front to back peeling, blending and final rendering
GLSLShader cubeShader, frontPeelShader, blendShader, finalShader;

//total number of depth peeling passes
const int NUM_PASSES=6;

//flag to use occlusion queries
bool bUseOQ = true;

//flag to use depth peeling 
bool bShowDepthPeeling = true;

//background colour
glm::vec4 bg=glm::vec4(0,0,0,0);

//FBO initialization function
void initFBO() {
	//generate 2 FBO
	glGenFramebuffers(2, fbo);
	//The FBO has two colour attachments
	glGenTextures (2, texID);
	//The FBO has two depth attachments
	glGenTextures (2, depthTexID);

	//for each attachment
	for(int i=0;i<2;i++) {
		//first initialize the depth texture
		glBindTexture(GL_TEXTURE_RECTANGLE, depthTexID[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE , 0, GL_DEPTH_COMPONENT32F, WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		//second initialize the colour attachment
		glBindTexture(GL_TEXTURE_RECTANGLE,texID[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE , 0,GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

		//bind FBO and attach the depth and colour attachments
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_RECTANGLE, depthTexID[i], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, texID[i], 0);
	}

	//Now setup the colour attachment for colour blend FBO
	glGenTextures(1, &colorBlenderTexID);
	glBindTexture(GL_TEXTURE_RECTANGLE, colorBlenderTexID);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);

	//generate the colour blend FBO ID
	glGenFramebuffers(1, &colorBlenderFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER, colorBlenderFBOID);

	//set the depth attachment of previous FBO as depth attachment for this FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, depthTexID[0], 0);
	//set the colour blender texture as the FBO colour attachment 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, colorBlenderTexID, 0);

	//check the FBO completeness status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE )
		printf("FBO setup successful !!! \n");
	else
		printf("Problem with FBO setup");

	//unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//delete all FBO related resources
void shutdownFBO() {
	glDeleteFramebuffers(2, fbo);
	glDeleteTextures (2, texID);
	glDeleteTextures (2, depthTexID);
	glDeleteFramebuffers(1, &colorBlenderFBOID);
	glDeleteTextures(1, &colorBlenderTexID);
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
		dist += (y - oldY)/5.0f;
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

	//initialize FBO
	initFBO();

	//generate hardwre query
	glGenQueries(1, &queryId);

	//create a uniform grid of size 20x20 in XZ plane
	grid = new CGrid(20,20);

	GL_CHECK_ERRORS

	//generate the quad vertices
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//generate quad indices
	GLushort quadIndices[]={ 0,1,2,0,2,3};

	//generate quad  vertex array and vertex buffer objects
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//pass quad vertices to buffer object memory
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS

		//enable vertex attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);

		//pass the quad indices to the element array buffer
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices[0], GL_STATIC_DRAW);

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

		//pass cube indices to element array  buffer
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cubeIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	//Load the cube shader
	cubeShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/cube_shader.vert");
	cubeShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/cube_shader.frag");

	//compile and link the shader
	cubeShader.CreateAndLinkProgram();
	cubeShader.Use();
		//add attributes and uniforms
		cubeShader.AddAttribute("vVertex");
		cubeShader.AddUniform("MVP");
		cubeShader.AddUniform("vColor");
	cubeShader.UnUse();

	//Load the front to back peeling shader
	frontPeelShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/front_peel.vert");
	frontPeelShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/front_peel.frag");
	//compile and link the shader
	frontPeelShader.CreateAndLinkProgram();
	frontPeelShader.Use();
		//add attributes and uniforms
		frontPeelShader.AddAttribute("vVertex");
		frontPeelShader.AddUniform("MVP");
		frontPeelShader.AddUniform("vColor");
		frontPeelShader.AddUniform("depthTexture");
		//pass constant uniforms at initialization
		glUniform1i(frontPeelShader("depthTexture"), 0);
	frontPeelShader.UnUse();

	//Load the blending shader
	blendShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/blend.vert");
	blendShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/blend.frag");
	//compile and link the shader
	blendShader.CreateAndLinkProgram();
	blendShader.Use();
		//add attributes and uniforms
		blendShader.AddAttribute("vVertex");
		blendShader.AddUniform("tempTexture");
		//pass constant uniforms at initialization
		glUniform1i(blendShader("tempTexture"), 0);
	blendShader.UnUse();

	//Load the final shader
	finalShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/blend.vert");
	finalShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/final.frag");
	//compile and link the shader
	finalShader.CreateAndLinkProgram();
	finalShader.Use();
		//add attributes and uniforms
		finalShader.AddAttribute("vVertex");
		finalShader.AddUniform("colorTexture");
		finalShader.AddUniform("vBackgroundColor");
		//pass constant uniforms at initialization
		glUniform1i(finalShader("colorTexture"), 0);
	finalShader.UnUse();
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	cubeShader.DeleteShaderProgram();
	frontPeelShader.DeleteShaderProgram();
	blendShader.DeleteShaderProgram();
	finalShader.DeleteShaderProgram();

	shutdownFBO();
	glDeleteQueries(1, &queryId);

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

	glDeleteVertexArrays(1, &cubeVAOID);
	glDeleteBuffers(1, &cubeVBOID);
	glDeleteBuffers(1, &cubeIndicesID);


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

//idle callback
void OnIdle() {
	//create a new rotation matrix for rotation on the Y axis
	R = glm::rotate(glm::mat4(1), glm::radians(angle+=5), glm::vec3(0,1,0));
	//recall the display callback
	glutPostRedisplay();
}

//function to render scene given the combined modelview projection matrix 
//and a shader
void DrawScene(const glm::mat4& MVP, GLSLShader& shader) {
	//enable alpha blending with over compositing
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//bind the cube vertex array object
	glBindVertexArray(cubeVAOID);
	//bind the shader
	shader.Use();
	//for all cubes
	for(int k=-1;k<=1;k++) {
		for(int j=-1;j<=1;j++) {
			int index =0;
			for(int i=-1;i<=1;i++) {
				GL_CHECK_ERRORS
				//set the modelling transformation and shader uniforms
				glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(i*2,j*2,k*2));
				glUniform4fv(shader("vColor"),1, &(box_colors[index++].x));
				glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP*R*T));
				//draw the cube
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
				GL_CHECK_ERRORS
			}
		}
	}
	//unbind shader
	shader.UnUse();
	//unbind vertex array object
	glBindVertexArray(0);
}

//function to draw a fullscreen quad
void DrawFullScreenQuad() {
	//bind the quad vertex array object
	glBindVertexArray(quadVAOID);
	//draw 2 triangles
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

//display callback function
void OnRender() {
	GL_CHECK_ERRORS

	//camera transformation
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//clear colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//get the combined modelview projection matrix
    glm::mat4 MVP	= P*MV;

	//if we want to use depth peeling 
	if(bShowDepthPeeling) {
		//bind the colour blending FBO
		glBindFramebuffer(GL_FRAMEBUFFER, colorBlenderFBOID);
		//set the first colour attachment as the draw buffer
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//clear the colour and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		// 1. In the first pass, we render normally with depth test enabled to get the nearest surface
		glEnable(GL_DEPTH_TEST);
		DrawScene(MVP, cubeShader);

		// 2. Depth peeling + blending pass
		int numLayers = (NUM_PASSES - 1) * 2;

		//for each pass
		for (int layer = 1; bUseOQ || layer < numLayers; layer++) {
			int currId = layer % 2;
			int prevId = 1 - currId;

			//bind the current FBO
			glBindFramebuffer(GL_FRAMEBUFFER, fbo[currId]);
			//set the first colour attachment as draw buffer
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			//set clear colour to black
			glClearColor(0, 0, 0, 0);
			//clear the colour and depth buffers
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//disbale blending and depth testing
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);

			//if we want to use occlusion query, we initiate it
			if (bUseOQ) {
				glBeginQuery(GL_SAMPLES_PASSED_ARB, queryId);
			}

			GL_CHECK_ERRORS

			//bind the depth texture from the previous step
			glBindTexture(GL_TEXTURE_RECTANGLE, depthTexID[prevId]);

			//render scene with the front to back peeling shader
	 		DrawScene(MVP, frontPeelShader);

			//if we initiated the occlusion query, we end it 
			if (bUseOQ) {
				glEndQuery(GL_SAMPLES_PASSED_ARB);
			}

			GL_CHECK_ERRORS

			//bind the colour blender FBO
			glBindFramebuffer(GL_FRAMEBUFFER, colorBlenderFBOID);
			//render to its first colour attachment 
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			//enable blending but disable depth testing
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);

			//change the blending equation to add
			glBlendEquation(GL_FUNC_ADD);
			//use separate blending function
			glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE,
								GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

			//bind the result from the previous iteration as texture
			glBindTexture(GL_TEXTURE_RECTANGLE, texID[currId]);
			//bind the blend shader and then draw a fullscreen quad
			blendShader.Use();
				 DrawFullScreenQuad();
			blendShader.UnUse();

			//disable blending
			glDisable(GL_BLEND);

			GL_CHECK_ERRORS

			//if we initiated the occlusion query, we get the query result
			//that is the total number of samples
			if (bUseOQ) {
				GLuint sample_count;
				glGetQueryObjectuiv(queryId, GL_QUERY_RESULT, &sample_count);
				if (sample_count == 0) {
					break;
				}
			}
		}

		GL_CHECK_ERRORS

		// 3. Final render pass
		//remove the FBO 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//restore the default back buffer
		glDrawBuffer(GL_BACK_LEFT);
		//disable depth testing and blending
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		//bind the colour blender texture
		glBindTexture(GL_TEXTURE_RECTANGLE, colorBlenderTexID);
		//bind the final shader
		finalShader.Use();
			//set shader uniforms
			glUniform4fv(finalShader("vBackgroundColor"), 1, &bg.x);
			//draw full screen quad
			DrawFullScreenQuad();
		finalShader.UnUse();

	} else {
		//no depth peeling, render scene with default alpha blending
		glEnable(GL_DEPTH_TEST);
		DrawScene(MVP, cubeShader);
	}

	//render grid
	grid->Render(glm::value_ptr(MVP));

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//Keyboard event handler to toggle the depth peeling usage
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ' ':
		bShowDepthPeeling = !bShowDepthPeeling;
			break;
	}
	if(bShowDepthPeeling)
		glutSetWindowTitle("Front-to-back Depth Peeling: On");
	else
		glutSetWindowTitle("Front-to-back Depth Peeling: Off");
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Front-to-back Depth Peeling - OpenGL 3.3");

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
	glutKeyboardFunc(OnKey);
	glutIdleFunc(OnIdle);

	//main loop call
	glutMainLoop();

	return 0;
}