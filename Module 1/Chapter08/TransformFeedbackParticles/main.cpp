
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "../src/GLSLShader.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>

using namespace std;
#pragma comment(lib, "glew32.lib")

#define CHECK_GL_ERRORS assert(glGetError()==GL_NO_ERROR);

const int width = 1024, height = 1024;

//max number of particles in the simulation
const int TOTAL_PARTICLES = 10000;

//total number of iterations for transform feedback
const int NUM_ITER = 1;

//variables for camera transformation
int oldX=0, oldY=0;
float rX=10, rY=0;
int state =1 ;
float dist=-11;

//the grid size
const int GRID_SIZE=10;

//information message buffer
char info[MAX_PATH]={0};

//timing variables
float timeStep =  1.0f/60.0f;
float currentTime = 0;
double accumulator = timeStep;
 
glm::mat4 mMVP;	//combined modelview projection matrix
glm::mat4 mMV;	//modelview matrix
glm::mat4 mP;	//projection matrix
 

//variables for high performance timer
LARGE_INTEGER frequency;        // ticks per second
LARGE_INTEGER t1, t2;           // ticks
double frameTimeQP=0;
float frameTime =0 ;

//for fps calculation
float startTime =0, fps=0 ;
int totalFrames=0;

//for transform feedback query
GLuint primitives_written=0;

//particle size
GLfloat pointSize = 10;

//colour constants
GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat vBeige[] = { 1.0f, 0.8f, 0.7f, 1.0f };
GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat vGray[] = { .25f, .25f, .25f, 1.0f };

//delta time per frame for transform feedback
float delta_time=0;

//ping pong ids
int readID=0, writeID = 1;

//buffer object ids for current and previous positions
GLuint	vboID_Pos[2],
		vboID_PrePos[2];

//vertex array object ids for update, render cycle 
GLuint vaoUpdateID[2], vaoRenderID[2];
GLuint vboID_Direction[2];
size_t i=0;

//shaders for particle, passthrough and rendering
GLSLShader	particleShader, passShader,
			renderShader;

//timer query ids
GLuint t_query, query;

//elapsed time per frame
GLuint64 elapsed_time;

//grid rendering variables
GLuint gridVAOID, gridVBOVerticesID, gridVBOIndicesID;
vector<glm::vec3> grid_vertices;
vector<GLushort> grid_indices;

//transform feedback id
GLuint tfID;

//current time
float t=0; 

const int HALF_RAND = (RAND_MAX / 2);

//particle variables
int max_life = 60;
int	  emitterLife = max_life;
int   emitterLifeVar = 15;

//function to convert degrees to radians
float DEGTORAD(const float f) {
	return f*(float)M_PI/180.0f;
}

//emitter variables and variation for each particle
float emitterYaw = DEGTORAD(0.0f);
float emitterYawVar	= DEGTORAD(360.0f);
float emitterPitch	= DEGTORAD(90.0f);
float emitterPitchVar = DEGTORAD(40.0f);
float emitterSpeed = 0.05f;
float emitterSpeedVar = 0.01f;

//pseudo random number generator
float RandomNum()
{
	int rn;
	rn = rand();
	return ((float)(rn - HALF_RAND) / (float)HALF_RAND);
}

//converts a given pitch and yaw value to a unit direction vector 
void RotationToDirection(float pitch,float yaw,glm::vec3 *direction)
{
	direction->x = -sin(yaw) * cos(pitch);
	direction->y = sin(pitch);
	direction->z = cos(pitch) * cos(yaw);
}

//creates buffer objects for particles
void createVBO()
{
	//fill the vertices
	int count = 0;

    // create vertex array objects
	glGenVertexArrays(2, vaoUpdateID);
	glGenVertexArrays(2, vaoRenderID);

	// create buffer objects
	glGenBuffers( 2, vboID_Pos);
	glGenBuffers( 2, vboID_PrePos);
  	glGenBuffers( 2, vboID_Direction);

	//set update vao
	for(int i=0;i<2;i++) {
		glBindVertexArray(vaoUpdateID[i]);
		//pass current positions 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Pos[i]);
		glBufferData( GL_ARRAY_BUFFER, TOTAL_PARTICLES* sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

		//xyz -> position
		//w -> speed
		//enable vertex attribute attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 0, 0);

		CHECK_GL_ERRORS

		//pass previous positions 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_PrePos[i]);
		glBufferData( GL_ARRAY_BUFFER, TOTAL_PARTICLES*sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

		//xyz -> previous position
		//w -> life
		//enable vertex attribute attribute
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,  4, GL_FLOAT, GL_FALSE, 0,0);

		CHECK_GL_ERRORS;

		//xyz -> initial direction
		//w -> life
		//pass initial particle directions 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Direction[i]);
		glBufferData( GL_ARRAY_BUFFER, TOTAL_PARTICLES*sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

		//enable vertex attribute attribute
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2,  4, GL_FLOAT, GL_FALSE, 0,0);

	}

	CHECK_GL_ERRORS;

	//setup the render vao which simply uses the position buffer object we filled earlier
	for(int i=0;i<2;i++) {
		glBindVertexArray(vaoRenderID[i]);
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Pos[i]);
		//enable vertex attribute array
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindVertexArray(0);

	//setup the grid vertices
	for(int i=-GRID_SIZE;i<=GRID_SIZE;i++)
	{
		grid_vertices.push_back(glm::vec3((float)i,0,(float)-GRID_SIZE));
		grid_vertices.push_back(glm::vec3((float)i,0,(float)GRID_SIZE));

		grid_vertices.push_back(glm::vec3((float)-GRID_SIZE,0,(float)i));
		grid_vertices.push_back(glm::vec3((float)GRID_SIZE,0,(float)i));
	}

	//fill the grid indices
	for(int i=0;i<GRID_SIZE*GRID_SIZE;i+=4) {
		grid_indices.push_back(i);
		grid_indices.push_back(i+1);
		grid_indices.push_back(i+2);
		grid_indices.push_back(i+3);
	}

	//Create grid VAO/VBO
	glGenVertexArrays(1, &gridVAOID);
	glGenBuffers (1, &gridVBOVerticesID);
	glGenBuffers (1, &gridVBOIndicesID);
	glBindVertexArray(gridVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, gridVBOVerticesID);
		//pass grid vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(float)*3*grid_vertices.size(), &grid_vertices[0].x, GL_STATIC_DRAW);
		//enable vertex attribute array
		glEnableVertexAttribArray(0);
		glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE,0,0);

		CHECK_GL_ERRORS
		//pass grid indices to element array buffer object
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridVBOIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*grid_indices.size(), &grid_indices[0], GL_STATIC_DRAW);


	glBindVertexArray(0);

    CHECK_GL_ERRORS;
}


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

//grid rendering routine 
//it uses the pass through shader with the given uniform colour as fragment colour 
void DrawGrid()
{
	passShader.Use();
		glUniform4fv(passShader("vColor"),1, vGray);
		glBindVertexArray(gridVAOID);
		glUniformMatrix4fv(passShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			glDrawElements(GL_LINES, grid_indices.size(),GL_UNSIGNED_SHORT,0);
		glBindVertexArray(0);
	passShader.UnUse();
}

//particle rendering routine
//use the render shader which uses the interpolated colour from the vertex shader as fragment colour
void RenderParticles()
{
	glBindVertexArray(vaoRenderID[readID]);
	renderShader.Use();
		glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			glDrawArrays(GL_POINTS, 0, TOTAL_PARTICLES);
	renderShader.UnUse();
	glBindVertexArray(0);
}

void InitGL() {
	//generate hardware queries
	glGenQueries(1, &query);
	glGenQueries(1, &t_query);

	CHECK_GL_ERRORS
	//get initial time
	startTime = (float)glutGet(GLUT_ELAPSED_TIME);
	// get ticks per second
    QueryPerformanceFrequency(&frequency);

    // start timer
    QueryPerformanceCounter(&t1);

	//enable blending with over compositing
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//setup shader loading
	particleShader.LoadFromFile(GL_VERTEX_SHADER,"shaders/Particle.vert");

	renderShader.LoadFromFile(GL_VERTEX_SHADER,"shaders/Render.vert");
	renderShader.LoadFromFile(GL_FRAGMENT_SHADER,"shaders/Render.frag");

	passShader.LoadFromFile(GL_VERTEX_SHADER,"shaders/Passthrough.vert");
	passShader.LoadFromFile(GL_FRAGMENT_SHADER,"shaders/Passthrough.frag");

	//compile and link particle shader
	particleShader.CreateAndLinkProgram();
	particleShader.Use();
		//add attribute and uniform locations
		particleShader.AddAttribute("position");
		particleShader.AddAttribute("prev_position");
		particleShader.AddAttribute("direction");
		particleShader.AddUniform("MVP");
		particleShader.AddUniform("time");
	particleShader.UnUse();

	//compile and link render shader
	renderShader.CreateAndLinkProgram();
	renderShader.Use();
		//add attribute and uniform locations
		renderShader.AddAttribute("position");
		renderShader.AddUniform("MVP");
	renderShader.UnUse();

	CHECK_GL_ERRORS

	//compile and link passthrough shader
	passShader.CreateAndLinkProgram();
	passShader.Use();
		//add attribute and uniform locations
		passShader.AddAttribute("position");
		passShader.AddUniform("MVP");
		passShader.AddUniform("vColor");
	passShader.UnUse();

	CHECK_GL_ERRORS

	//create vbo
	createVBO();

	//setup transform feedback attributes
	glGenTransformFeedbacks(1, &tfID);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfID);
	//pass the vertex shader outputs for transform feedback
	const char* varying_names[]={"out_position", "out_prev_position", "out_direction"};
	glTransformFeedbackVaryings(particleShader.GetProgram(), 3, varying_names, GL_SEPARATE_ATTRIBS);
	//relink the particle shader program
	glLinkProgram(particleShader.GetProgram());

	//set the particle size
	glPointSize(pointSize); 
}

//resize event handler
void OnReshape(int nw, int nh) {
	//reset the viewport
	glViewport(0,0,nw, nh);

	//get the perspective projection matrix
	mP = glm::perspective(60.0f, (GLfloat)nw/nh, 1.0f, 100.f);
  
}

//update the particle position on GPU
void UpdateParticlesGPU() {
	//get current elapsed time
	t = (float)glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	
	//set the particle shader
	particleShader.Use();
		//pass shader uniforms
		glUniformMatrix4fv(particleShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
		glUniform1f(particleShader("time"), t);
		CHECK_GL_ERRORS
		//run the iteration loop
		for(int i=0;i<NUM_ITER;i++) {
			//set the update vertex array object
			glBindVertexArray( vaoUpdateID[readID]);
				//bind transform feedback buffers
				//index 0 -> current position
				//index 1 -> previous position
				//index 2 -> particle direction
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vboID_Pos[writeID]);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vboID_PrePos[writeID]);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, vboID_Direction[writeID]);
				//disable rasterizer
				glEnable(GL_RASTERIZER_DISCARD);    
					//run hardware timer query
					glBeginQuery(GL_TIME_ELAPSED,t_query);
						//initiate transform feedback
						glBeginTransformFeedback(GL_POINTS);
							//render points, this call pushes all attributes to GPU
							glDrawArrays(GL_POINTS, 0, TOTAL_PARTICLES);
						//end transform feedback
						glEndTransformFeedback();
					//end timer query
					glEndQuery(GL_TIME_ELAPSED);
					glFlush();
				//enable rasterizer
				glDisable(GL_RASTERIZER_DISCARD);

			//swap read/write pathways
			int tmp = readID;
			readID  = writeID;
			writeID = tmp;
		}
		CHECK_GL_ERRORS
		// get the query result
		glGetQueryObjectui64v(t_query, GL_QUERY_RESULT, &elapsed_time);
		//get the transform feedback time
		delta_time = elapsed_time / 1000000.0f;
	//remove the particle shader
	particleShader.UnUse();

	CHECK_GL_ERRORS
}

//display function
void OnRender() {

	CHECK_GL_ERRORS

	//timing related function calls
	float newTime = (float) glutGet(GLUT_ELAPSED_TIME);
	frameTime = newTime-currentTime;
	currentTime = newTime; 

	//Using high res. counter
    QueryPerformanceCounter(&t2);
	 // compute and print the elapsed time in millisec
    frameTimeQP = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	t1=t2;
	accumulator += frameTimeQP;

	++totalFrames;

	//FPS calculation code
	if((newTime-startTime)>1000)
	{
		float elapsedTime = (newTime-startTime);
		fps = (totalFrames/ elapsedTime)*1000 ;
		startTime = newTime;
		totalFrames=0;
		sprintf_s(info, "FPS: %3.2f, Frame time (GLUT): %3.4f msecs, Frame time (QP): %3.3f, TF Time: %3.3f", fps , frameTime, frameTimeQP, delta_time);
	}
	
	glutSetWindowTitle(info);

	//clear colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	//set the camera transform
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	mMV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	mMVP = mP*mMV;
	 
	//draw grid
 	DrawGrid();

	//update particles on GPU
	UpdateParticlesGPU();

	//render particles
	RenderParticles();

	//swap back and front buffers to display the result on screen
	glutSwapBuffers();
}

//delete all allocated objects
void OnShutdown() {

	glDeleteQueries(1, &query);
	glDeleteQueries(1, &t_query);

	glDeleteVertexArrays(2, vaoUpdateID);
	glDeleteVertexArrays(2, vaoRenderID);

	glDeleteVertexArrays(1, &gridVAOID);
	glDeleteBuffers( 1, &gridVBOVerticesID);
	glDeleteBuffers( 1, &gridVBOIndicesID);

    glDeleteBuffers( 2, vboID_Pos);
	glDeleteBuffers( 2, vboID_PrePos);
	glDeleteBuffers( 2, vboID_Direction);

	glDeleteTransformFeedbacks(1, &tfID);
	renderShader.DeleteShaderProgram();
	particleShader.DeleteShaderProgram();
	passShader.DeleteShaderProgram();

	printf("Shutdown successful.");
}

void OnIdle() {
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitContextVersion(3,3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Simple Particle System using Transform Feedback");

	//callback hooks
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnReshape);
	glutIdleFunc(OnIdle);

	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutCloseFunc(OnShutdown);

	//glew initialization
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
	}

	GLuint error = glGetError();

	// Only continue, if OpenGL 3.3 is supported.
	if (!glewIsSupported("GL_VERSION_3_3"))
	{
  		puts("OpenGL 3.3 not supported.");
		exit(EXIT_FAILURE);
	} else {
		puts("OpenGL 3.3 supported.");
	}
	if(!glewIsExtensionSupported("GL_ARB_transform_feedback2"))
	{
		puts("Your hardware does not support a required extension [GL_ARB_transform_feedback2].");
		exit(EXIT_FAILURE);
	} else {
		puts("GL_ARB_transform_feedback2 supported.");
	}
	//output information on screen
	printf("Using GLEW %s\n",glewGetString(GLEW_VERSION));
	printf("Vendor: %s\n",glGetString (GL_VENDOR));
	printf("Renderer: %s\n",glGetString (GL_RENDERER));
	printf("Version: %s\n",glGetString (GL_VERSION));
	printf("GLSL: %s\n",glGetString (GL_SHADING_LANGUAGE_VERSION));

	//initialization of OpenGL
	InitGL();

	//main loop call
	glutMainLoop();

	return 0;
}

