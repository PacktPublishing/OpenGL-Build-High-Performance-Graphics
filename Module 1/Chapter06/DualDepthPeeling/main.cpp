#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

const float MAX_DEPTH = 1.0f;

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

//dual depth peeling FBO id
GLuint dualDepthFBOID;
//back texture colour attachment IDs
GLuint backTexID[2];
//front texture colour attachment IDs
GLuint texID[2];
//back texture depth attachment IDs
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

//shaders for cube, initialization, dual depth peeling, blending and final rendering
GLSLShader cubeShader, initShader, dualPeelShader, blendShader, finalShader;

//total number of depth peeling passes
const int NUM_PASSES=4;

//flag to use occlusion queries
bool bUseOQ = true;

//flag to use dual depth peeling
bool bShowDepthPeeling = true;

//blending colour alpha
float alpha=0.6f;

//background colour
glm::vec4 bg=glm::vec4(0,0,0,0);

//colour attachment IDs
GLenum attachID[2]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT3};

//draw buffer attachments
GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
						   GL_COLOR_ATTACHMENT1,
						   GL_COLOR_ATTACHMENT2,
						   GL_COLOR_ATTACHMENT3,
						   GL_COLOR_ATTACHMENT4,
						   GL_COLOR_ATTACHMENT5,
						   GL_COLOR_ATTACHMENT6
};

//FBO initialization function
void initFBO() {
	//generate dual depth FBO
	glGenFramebuffers(1, &dualDepthFBOID);
	//The FBO has 4 colour attachments
	glGenTextures (2, texID);
	glGenTextures (2, backTexID);
	//The FBO has 2 depth attachments
	glGenTextures (2, depthTexID);

	//for each attachment
	for(int i=0;i<2;i++) {
		//first initialize the depth texture
		glBindTexture(GL_TEXTURE_RECTANGLE, depthTexID[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE , 0, GL_FLOAT_RG32_NV, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);

		//initialize the colour attachment
		glBindTexture(GL_TEXTURE_RECTANGLE,texID[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE , 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

		//initialize the back colour attachment
		glBindTexture(GL_TEXTURE_RECTANGLE,backTexID[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE , 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	GL_CHECK_ERRORS

	//Now setup the colour attachment for colour blend FBO
	glGenTextures(1, &colorBlenderTexID);
	glBindTexture(GL_TEXTURE_RECTANGLE, colorBlenderTexID);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, 0);

	//generate the colour blend FBO ID
	glGenFramebuffers(1, &colorBlenderFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER, colorBlenderFBOID);
	//set the colour blender texture as the FBO colour attachment 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, colorBlenderTexID, 0);

	//bind the dual depth FBO
	glBindFramebuffer(GL_FRAMEBUFFER, dualDepthFBOID);

	//bind the six colour attachments for this FBO
	for(int i=0;i<2;i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachID[i], GL_TEXTURE_RECTANGLE, depthTexID[i], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachID[i]+1, GL_TEXTURE_RECTANGLE, texID[i], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachID[i]+2, GL_TEXTURE_RECTANGLE, backTexID[i], 0);
	}

	//set the colour blender texture as the 7th attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_RECTANGLE, colorBlenderTexID, 0);
		
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
	glDeleteFramebuffers(1, &dualDepthFBOID);
	glDeleteFramebuffers(1, &colorBlenderFBOID);
	glDeleteTextures (2, texID);
	glDeleteTextures (2, backTexID);
	glDeleteTextures (2, depthTexID);
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

	GL_CHECK_ERRORS

	//Load the initialization shader
	initShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/cube_shader.vert");
	initShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/dual_init.frag");
	//compile and link the shader
	initShader.CreateAndLinkProgram();
	initShader.Use();
		//add attributes and uniforms
		initShader.AddAttribute("vVertex");
		initShader.AddUniform("MVP");
	initShader.UnUse();

	GL_CHECK_ERRORS

	//Load the dual depth peeling shader
	dualPeelShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/dual_peel.vert");
	dualPeelShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/dual_peel.frag");
	//compile and link the shader
	dualPeelShader.CreateAndLinkProgram();
	dualPeelShader.Use();
		//add attributes and uniforms
		dualPeelShader.AddAttribute("vVertex");
		dualPeelShader.AddUniform("MVP");
		dualPeelShader.AddUniform("vColor");
		dualPeelShader.AddUniform("alpha");
		dualPeelShader.AddUniform("depthBlenderTex");
		dualPeelShader.AddUniform("frontBlenderTex");
		//pass constant uniforms at initialization
		glUniform1i(dualPeelShader("depthBlenderTex"), 0);
		glUniform1i(dualPeelShader("frontBlenderTex"), 1);
	dualPeelShader.UnUse();

	GL_CHECK_ERRORS

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

	GL_CHECK_ERRORS

	//Load the final shader
	finalShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/blend.vert");
	finalShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/final.frag");
	//compile and link the shader
	finalShader.CreateAndLinkProgram();
	finalShader.Use();
		//add attributes and uniforms
		finalShader.AddAttribute("vVertex");
		finalShader.AddUniform("depthBlenderTex");
		finalShader.AddUniform("frontBlenderTex");
		finalShader.AddUniform("backBlenderTex");
		//pass constant uniforms at initialization
		glUniform1i(finalShader("depthBlenderTex"), 0);
		glUniform1i(finalShader("frontBlenderTex"), 1);
		glUniform1i(finalShader("backBlenderTex"), 2);
	finalShader.UnUse();

	GL_CHECK_ERRORS

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	cubeShader.DeleteShaderProgram();
	initShader.DeleteShaderProgram();
	dualPeelShader.DeleteShaderProgram();
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
void DrawScene(const glm::mat4& MVP, GLSLShader& shader, bool useColor=false, bool useAlphaMultiplier=false) {
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
				if(useColor)
					glUniform4fv(shader("vColor"),1, &(box_colors[index++].x));
				if(useAlphaMultiplier)
					glUniform1f(shader("alpha"), alpha);

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
	//diable alpha blending
	glDisable(GL_BLEND);
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
		
		//disble depth test and enable alpha blending
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		//bind dual depth FBO
		glBindFramebuffer(GL_FRAMEBUFFER, dualDepthFBOID);

		// Render targets 1 and 2 store the front and back colors
		// Clear to 0.0 and use MAX blending to filter written color
		// At most one front color and one back color can be written every pass
		glDrawBuffers(2, &drawBuffers[1]);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		GL_CHECK_ERRORS

		// Render target 0 stores (-minDepth, maxDepth)
		glDrawBuffer(drawBuffers[0]);
		//clear the offscreen texture with -MAX_DEPTH
		glClearColor(-MAX_DEPTH, -MAX_DEPTH, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		//enable max blending
		glBlendEquation(GL_MAX);
		//render scene with the initialization shader
		DrawScene(MVP, initShader);

		// 2. Depth peeling + blending pass
		glDrawBuffer(drawBuffers[6]);
		//clear color buffer with the background colour
		glClearColor(bg.x, bg.y, bg.z, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		 
		int currId = 0;
		//for each pass
		for (int layer = 1; bUseOQ || layer < NUM_PASSES; layer++) {
			currId = layer % 2;
			int prevId = 1 - currId;
			int bufId = currId * 3;

			//render to 2 colour attachments simultaneously
			glDrawBuffers(2, &drawBuffers[bufId+1]);
			//set clear color to black and clear colour buffer
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			//alternate the colour attachment for draw buffer
			glDrawBuffer(drawBuffers[bufId+0]);
			//clear the color to -MAX_DEPTH and clear colour buffer
			glClearColor(-MAX_DEPTH, -MAX_DEPTH, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			//Render to three draw buffers simultaneously
			// Render target 0: RG32F MAX blending
			// Render target 1: RGBA MAX blending
			// Render target 2: RGBA MAX blending
			glDrawBuffers(3, &drawBuffers[bufId+0]);	
			//enable max blending
			glBlendEquation(GL_MAX);

			//bind depth texture to texture unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, depthTexID[prevId]);

			//bind colour attachment texture to texture unit 1
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_RECTANGLE, texID[prevId]);
			
			//draw scene using the dual peel shader 
			DrawScene(MVP, dualPeelShader, true,true);

			// Full screen pass to alpha-blend the back color
			glDrawBuffer(drawBuffers[6]);

			//set the over blending 
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			 
			//if we want to use occlusion query, we initiate it
			if (bUseOQ) {
				glBeginQuery(GL_SAMPLES_PASSED_ARB, queryId);
			}

			GL_CHECK_ERRORS

			//bind the back colour attachment to texture unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, backTexID[currId]);

			//use blending shader and draw a fullscreen quad
			blendShader.Use();
				 DrawFullScreenQuad();
			blendShader.UnUse();

			//if we initiated the occlusion query, we end it and get
			//the query result which is the total number of samples
			//output from the blending result
			if (bUseOQ) {
				glEndQuery(GL_SAMPLES_PASSED);
				GLuint sample_count;
				glGetQueryObjectuiv(queryId, GL_QUERY_RESULT, &sample_count);
				if (sample_count == 0) {
					break;
				}
			}
			GL_CHECK_ERRORS
		}

		GL_CHECK_ERRORS

		//disable alpha blending
		glDisable(GL_BLEND);

		// 3. Final render pass
		//remove the FBO 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//restore the default back buffer
		glDrawBuffer(GL_BACK_LEFT);
		 
		//bind the depth texture to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, depthTexID[currId]);

		//bind the depth texture to colour texture to texture unit 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_RECTANGLE, texID[currId]);
		
		//bind the colour blender texture to texture unit 2
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_RECTANGLE, colorBlenderTexID);

		//bind the final shader and draw a fullscreen quad
		finalShader.Use();
			DrawFullScreenQuad();
		finalShader.UnUse();

	} else {
		//no depth peeling, render scene with default alpha blending
		glEnable(GL_DEPTH_TEST);
		DrawScene(MVP, cubeShader, true,false);
	}

	//render grid
	grid->Render(glm::value_ptr(MVP));

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//Keyboard event handler to toggle the dual depth peeling usage
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ' ':
		bShowDepthPeeling = !bShowDepthPeeling;
			break;
	}
	if(bShowDepthPeeling)
		glutSetWindowTitle("Dual Depth Peeling: On");
	else
		glutSetWindowTitle("Dual Depth Peeling: Off");
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Dual Depth Peeling - OpenGL 3.3");

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