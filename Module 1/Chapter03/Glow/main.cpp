
#define _USE_MATH_DEFINES
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

//screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//camera tranformation variables
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, fov = 45;


#include "..\src\FreeCamera.h"

//virtual key codes
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//espsilon epsilon2 for accuracy
const float EPSILON = 0.001f;
const float EPSILON2 = EPSILON*EPSILON;

//delta time
float dt = 0;

//Free camera instance
CFreeCamera cam;

//output message
#include <sstream>
std::stringstream msg;

//Grid object
#include "..\src\Grid.h"
CGrid* grid;

//Unit cube object
#include "..\src\UnitCube.h"
CUnitCube* cube;

//modelview, projection and rotation matrices
glm::mat4 MV,P;
glm::mat4 Rot;

//timing related variables
float last_time=0, current_time=0;

//mouse filtering support variables
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;

//mouse history buffer
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //filtered mouse values

//flag to enable filtering
bool useFiltering = true;

//distance of the points
float radius = 2;

//particle positions
glm::vec3 particles[8];

//particle vertex array and vertex buffer objects IDs
GLuint particlesVAO;
GLuint particlesVBO;

//particle and blurring shader
GLSLShader particleShader;
GLSLShader blurShader;

//quad vertex array and vertex buffer object IDs
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadVBOIndicesID;

//autorotate angle
float angle = 0;

//FBO ID
GLuint fboID;
//FBO colour attachment textures
GLuint texID[2]; //0 -> glow rendered output
				 //1 -> blurred output

//width and height of the FBO colour attachment
const int RENDER_TARGET_WIDTH = WIDTH>>1;
const int RENDER_TARGET_HEIGHT = HEIGHT>>1;

//mouse move filtering function
void filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    // Store current mouse entry at front of array.
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    // Filter the mouse.
    for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE; ++i)
    {
		glm::vec2 tmp=mouseHistory[i];
        averageX += tmp.x * currentWeight;
        averageY += tmp.y * currentWeight;
        averageTotal += 1.0f * currentWeight;
        currentWeight *= MOUSE_FILTER_WEIGHT;
    }

    mouseX = averageX / averageTotal;
    mouseY = averageY / averageTotal;

}

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
	if (state == 0) {
		fov += (y - oldY)/5.0f;
		cam.SetupProjection(fov, cam.GetAspectRatio());
	} else {
		rY += (y - oldY)/5.0f;
		rX += (oldX-x)/5.0f;
		if(useFiltering)
			filterMouseMoves(rX, rY);
		else {
			mouseX = rX;
			mouseY = rY;
		}
		cam.Rotate(mouseX,mouseY, 0);
	}
	oldX = x;
	oldY = y;
	glutPostRedisplay();
}

//initialize OpenGL
void OnInit() {

	//create grid object
	grid = new CGrid(20,20);

	//create unit cube
	cube = new CUnitCube();
	//set the cube colour as blue
	cube->color = glm::vec3(0,0,1);

	GL_CHECK_ERRORS

	//set the camera position
	glm::vec3 p = glm::vec3(5,5,5);
	cam.SetPosition(p);

	//orient the camera
	glm::vec3 look=  glm::normalize(p);
	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;
	if(useFiltering) {
		for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
			mouseHistory[i] = glm::vec2(rX, rY);
		}
	}
	cam.Rotate(rX,rY,0);

	//enable depth testing
	glEnable(GL_DEPTH_TEST);

	//set point size
	glPointSize(50);

	//load particle shader
	particleShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/particle.vert");
	particleShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/particle.frag");
	//compile and link shader
	particleShader.CreateAndLinkProgram();
	particleShader.Use();
		//set shader attributes
		particleShader.AddAttribute("vVertex");
		particleShader.AddUniform("MVP");
	particleShader.UnUse();

	//set particle positions
	for(int i=0;i<8;i++) {
		float theta = float(i/8.0f * 2 * M_PI);
		particles[i].x = radius*cos(theta);
		particles[i].y = 0.0f;
		particles[i].z = radius*sin(theta);
	}

	//set particle vertex array and vertex buffer objects
	glGenVertexArrays(1, &particlesVAO);
	glGenBuffers(1, &particlesVBO);
	glBindVertexArray(particlesVAO);

		glBindBuffer (GL_ARRAY_BUFFER, particlesVBO);
		//pass particle vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(particles), &particles[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(particleShader["vVertex"]);
		glVertexAttribPointer(particleShader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);

		GL_CHECK_ERRORS

	//setup FBO and offscreen render target
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

	//setup two colour attachments
	glGenTextures(2, texID);
	glActiveTexture(GL_TEXTURE0);
	for(int i=0;i<2;i++) {		
		glBindTexture(GL_TEXTURE_2D, texID[i]);
		//set texture parameters
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//allocate OpenGL texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		GL_CHECK_ERRORS

		//set the current texture as the FBO attachment 
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, texID[i], 0);
	}
	
	GL_CHECK_ERRORS

	//check for framebuffer completeness
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		cerr<<"Frame buffer object setup error."<<endl;
		exit(EXIT_FAILURE);
	} else {
		cerr<<"FBO setup successfully."<<endl;
	}

	//unbind the FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	GL_CHECK_ERRORS

	//setup fullscreen quad vertices
	glm::vec2 vertices[4];
	vertices[0]=glm::vec2(0,0);
	vertices[1]=glm::vec2( 1,0);
	vertices[2]=glm::vec2( 1, 1);
	vertices[3]=glm::vec2(0, 1);

	GLushort indices[6];

	int count = 0;

	//fill quad indices array
	GLushort* id=&indices[0];
	*id++ = 0; 	*id++ = 1; 	*id++ = 2;
	*id++ = 0; 	*id++ = 2; 	*id++ = 3;

	//load the blur shader
	blurShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/full_screen_shader.vert");
	blurShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/full_screen_shader.frag");
	//compile and link the shader
	blurShader.CreateAndLinkProgram();
	blurShader.Use();
		//add shader attributes and uniforms
		blurShader.AddAttribute("vVertex");
		blurShader.AddUniform("textureMap");
		//set the values of the constant uniforms at initialization
		glUniform1i(blurShader("textureMap"),0);
	blurShader.UnUse();

	GL_CHECK_ERRORS

	//set up quad vertex array and vertex buffer object
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadVBOIndicesID);

	glBindVertexArray(quadVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//pass quad vertices
		glBufferData (GL_ARRAY_BUFFER, 4*sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);

		//enable vertex attribute array for vertex position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);

		//pass the quad indices to element array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadVBOIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*6, &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	GL_CHECK_ERRORS
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	particleShader.DeleteShaderProgram();
	blurShader.DeleteShaderProgram();

	delete grid;
	delete cube;

	glDeleteBuffers(1, &particlesVBO);
	glDeleteBuffers(1, &particlesVAO);

	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadVBOIndicesID);
	glDeleteBuffers(1, &quadVAOID);

	glDeleteTextures(2, texID);
	glDeleteFramebuffers(1, &fboID);

	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport size
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//set the camera projection settings
	cam.SetupProjection(fov, (GLfloat)w/h);
}

//idle callback function
void OnIdle() {
	//generate a rotation matrix to rotate on the Y axis each idle event
	Rot = glm::rotate(glm::mat4(1), angle++, glm::vec3(0,1,0));

	//handle the WSAD QZ key events to move the camera around
	if( GetAsyncKeyState(VK_W) & 0x8000) {
		cam.Walk(dt);
	}

	if( GetAsyncKeyState(VK_S) & 0x8000) {
		cam.Walk(-dt);
	}

	if( GetAsyncKeyState(VK_A) & 0x8000) {
		cam.Strafe(-dt);
	}

	if( GetAsyncKeyState(VK_D) & 0x8000) {
		cam.Strafe(dt);
	}

	if( GetAsyncKeyState(VK_Q) & 0x8000) {
		cam.Lift(dt);
	}

	if( GetAsyncKeyState(VK_Z) & 0x8000) {
		cam.Lift(-dt);
	}
	glm::vec3 t = cam.GetTranslation(); 

	if(glm::dot(t,t)>EPSILON2) {
		cam.SetTranslation(t*0.95f);
	}

	//call the display function
	glutPostRedisplay();
}

//display callback function
void OnRender() {
	static int total = 0;
	static int offset = 0;
	total++;

	if(total%50 == 0)
	{
		offset = 4-offset;
	}
	GL_CHECK_ERRORS

	//timing related calcualtion
	last_time = current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	dt = current_time-last_time;

	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//setup the modelview and projection matrices and get the combined modelview projection matrix
	MV	= cam.GetViewMatrix();
	P   = cam.GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

	//Render scene normally
	//render the grid
	grid->Render(glm::value_ptr(MVP));

	//render the cube
	cube->Render(glm::value_ptr(MVP));

	//set the particle vertex array object
	glBindVertexArray(particlesVAO);
		//set the particle shader
		particleShader.Use();
			//set the shader uniforms
			glUniformMatrix4fv(particleShader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP*Rot));
				//draw particles
	     		glDrawArrays(GL_POINTS, 0, 8);

	 GL_CHECK_ERRORS


	//Activate FBO and render the scene elements which need glow
	//in our example, we will apply glow to the first 4 points
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
	
	//set the viewport to the size of the offscreen render target
	glViewport(0,0,RENDER_TARGET_WIDTH,RENDER_TARGET_HEIGHT);

	//set colour attachment 0 as the draw buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//clear the colour buffer
		glClear(GL_COLOR_BUFFER_BIT);
			//render 4 points
			glDrawArrays(GL_POINTS, offset, 4);
		//unbind the particle shader
		particleShader.UnUse();
	GL_CHECK_ERRORS

	//set the first colour attachment
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	//bind the output of the previous step as texture
	glBindTexture(GL_TEXTURE_2D, texID[0]);
		//use the blur shader
		blurShader.Use();
			//bind the fullscreen quad vertex array
			glBindVertexArray(quadVAOID);
				//render fullscreen quad
				glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,0);

	GL_CHECK_ERRORS

	//unbind the FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//restore the default back buffer
	glDrawBuffer(GL_BACK_LEFT);
	//bind the filtered texture from the final step
	glBindTexture(GL_TEXTURE_2D, texID[1]);

	GL_CHECK_ERRORS

	//reset the default viewport
	glViewport(0,0,WIDTH, HEIGHT);
	GL_CHECK_ERRORS
	//enable additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
		//draw fullscreen quad
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,0);
	glBindVertexArray(0);

	//unbind the blur shader
	blurShader.UnUse();

	//disable blending
	glDisable(GL_BLEND);

	GL_CHECK_ERRORS
	
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
	glutCreateWindow("Glow - OpenGL 3.3");

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
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutIdleFunc(OnIdle);

	//call main loop
	glutMainLoop();

	return 0;
}