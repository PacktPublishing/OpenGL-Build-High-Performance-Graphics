#include <GL/glew.h>

#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//screen size
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shaders for this recipe
GLSLShader shader;
//point rendering shader
GLSLShader pointShader;

//vertex array and vertex buffer objects
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//view frustum vertex array and vertex buffer object
GLuint vaoFrustumID;
GLuint vboFrustumVerticesID;
GLuint vboFrustumIndicesID;

const int NUM_X = 40; //total quads on X axis
const int NUM_Z = 40; //total quads on Z axis

const float SIZE_X = 100; //size of plane in world space
const float SIZE_Z = 100;
const float HALF_SIZE_X = SIZE_X/2.0f;
const float HALF_SIZE_Z = SIZE_Z/2.0f;

//total vertices and indices
glm::vec3 vertices[(NUM_X+1)*(NUM_Z+1)];
const int TOTAL_INDICES = NUM_X*NUM_Z*2*3;
GLushort indices[TOTAL_INDICES];

//for floating point imprecision
const float EPSILON = 0.001f;
const float EPSILON2 = EPSILON*EPSILON;

//camera tranformation variables
int state = 0, oldX=0, oldY=0;
float rX=-135, rY=45, fov = 45;

#include "..\..\src\FreeCamera.h"
 
//virtual key codes
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//delta time
float dt = 0;

//timing related variables
float last_time=0, current_time =0;


//2 FreeCamera instances and a current pointer
CFreeCamera cam;
CFreeCamera world;
CFreeCamera* pCurrentCam;

//mouse filtering support variables
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;

//mouse history buffer
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //filtered mouse values

//flag to enable filtering
bool useFiltering = true;

//view frustum vertices
glm::vec3 frustum_vertices[8];

//constant colours
GLfloat white[4] = {1,1,1,1};
GLfloat red[4] = {1,0,0,0.5};
GLfloat cyan[4] = {0,1,1,0.5};

//points
const int PX = 100;
const int PZ = 100;
const int MAX_POINTS=PX*PZ;

//point vertices, vertex array and vertex buffer objects
glm::vec3 pointVertices[MAX_POINTS];
GLuint pointVAOID, pointVBOID;


//number of points visible
int total_visible=0;

//hardware query 
GLuint query;

//FPS related variables
float start_time = 0;
float fps=0;
int total_frames;
float last_time_fps=0;

//string buffer for message display
char buffer[MAX_PATH]={'\0'};

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
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1;
}

//mouse move handler
void OnMouseMove(int x, int y)
{
	bool changed = false;
	if (state == 0) {
		fov += (y - oldY)/5.0f;
		pCurrentCam->SetFOV(fov);
		changed = true;
	} else if(state == 1) {
		rY += (y - oldY)/5.0f;
		rX += (oldX-x)/5.0f;
		if(useFiltering)
			filterMouseMoves(rX, rY);
		else {
			mouseX = rX;
			mouseY = rY;
		}
		if(pCurrentCam == &world) {
			cam.Rotate(mouseX, mouseY,0);
			cam.CalcFrustumPlanes();
		} else {
			pCurrentCam->Rotate(mouseX,mouseY, 0);
		}
		changed = true;
	}
	oldX = x;
	oldY = y;

	if(changed) {
		pCurrentCam->CalcFrustumPlanes();
		frustum_vertices[0] = cam.farPts[0];
		frustum_vertices[1] = cam.farPts[1];
		frustum_vertices[2] = cam.farPts[2];
		frustum_vertices[3] = cam.farPts[3];

		frustum_vertices[4] = cam.nearPts[0];
		frustum_vertices[5] = cam.nearPts[1];
		frustum_vertices[6] = cam.nearPts[2];
		frustum_vertices[7] = cam.nearPts[3];

		//update the frustum vertices on the GPU
		glBindVertexArray(vaoFrustumID);
			glBindBuffer (GL_ARRAY_BUFFER, vboFrustumVerticesID);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(frustum_vertices), &frustum_vertices[0]);
		glBindVertexArray(0);
	}

	glutPostRedisplay();
}
void OnInit() {
	//generate hardware query
	glGenQueries(1, &query);

	//enable polygin line drawing mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//enable depth testing
	glEnable(GL_DEPTH_TEST);

	//set point size
	glPointSize(10);

	GL_CHECK_ERRORS

	//load the shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
		//add attributes and uniforms
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP");
		shader.AddUniform("color");
		//set values of constant uniforms at initialization
		glUniform4fv(shader("color"),1,white);
	shader.UnUse();

	GL_CHECK_ERRORS

	//setup point rendering shader
	pointShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/points.vert");
	pointShader.LoadFromFile(GL_GEOMETRY_SHADER, "shaders/points.geom");
	pointShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/points.frag");
	//compile and link shader
	pointShader.CreateAndLinkProgram();
	pointShader.Use();
		//add attributes and uniforms
		pointShader.AddAttribute("vVertex");
		pointShader.AddUniform("MVP");
		pointShader.AddUniform("t");
		pointShader.AddUniform("FrustumPlanes");
	pointShader.UnUse();

	GL_CHECK_ERRORS

	//setup ground plane geometry
	//setup vertices
	int count = 0;
	int i=0, j=0;
	for( j=0;j<=NUM_Z;j++) {
		for( i=0;i<=NUM_X;i++) {
			vertices[count++] = glm::vec3( ((float(i)/(NUM_X-1)) *2-1)* HALF_SIZE_X, 0, ((float(j)/(NUM_Z-1))*2-1)*HALF_SIZE_Z);
		}
	}

	//fill indices array
	GLushort* id=&indices[0];
	for (i = 0; i < NUM_Z; i++) {
		for (j = 0; j < NUM_X; j++) {
			int i0 = i * (NUM_X+1) + j;
			int i1 = i0 + 1;
			int i2 = i0 + (NUM_X+1);
			int i3 = i2 + 1;
			if ((j+i)%2) {
				*id++ = i0; *id++ = i2; *id++ = i1;
				*id++ = i1; *id++ = i2; *id++ = i3;
			} else {
				*id++ = i0; *id++ = i2; *id++ = i3;
				*id++ = i0; *id++ = i3; *id++ = i1;
			}
		}
	}

	GL_CHECK_ERRORS

	//setup ground plane vao and vbo stuff
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//pass vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enabel vertex attribute array for position
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//set indices to element arrary buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS

	//setup camera position and rotate it
	cam.SetPosition(glm::vec3(2,2,2));
	cam.Rotate(rX,rY,0);
	//setup camera projection settings and update the matrices
	cam.SetupProjection(fov, ((GLfloat)WIDTH)/HEIGHT,1.f,10);
	cam.Update();

	//calculate the camera view furstum planes
	cam.CalcFrustumPlanes();

	//set the world camera position and direction
	world.SetPosition(glm::vec3(10,10,10));
	world.Rotate(rX,rY,0);
	//set the projection and update the camera settings
	world.SetupProjection(fov,(GLfloat)WIDTH/HEIGHT, 0.1f, 100.0f);
	world.Update();

	//assign the object cam as the current camera
	pCurrentCam = &cam;

	//setup Frustum geometry
	glGenVertexArrays(1, &vaoFrustumID);
	glGenBuffers(1, &vboFrustumVerticesID);
	glGenBuffers(1, &vboFrustumIndicesID);

	//store the view frustum vertices
	frustum_vertices[0] = cam.farPts[0];
	frustum_vertices[1] = cam.farPts[1];
	frustum_vertices[2] = cam.farPts[2];
	frustum_vertices[3] = cam.farPts[3];

	frustum_vertices[4] = cam.nearPts[0];
	frustum_vertices[5] = cam.nearPts[1];
	frustum_vertices[6] = cam.nearPts[2];
	frustum_vertices[7] = cam.nearPts[3];

	GLushort frustum_indices[36]={0,4,3,3,4,7, //top
								  6,5,1,6,1,2, //bottom
								  0,1,4,4,1,5, //left
								  7,6,3,3,6,2, //right
								  4,5,6,4,6,7, //near
								  3,2,0,0,2,1, //far
								  };
	glBindVertexArray(vaoFrustumID);

		glBindBuffer (GL_ARRAY_BUFFER, vboFrustumVerticesID);
		//pass frustum vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(frustum_vertices), &frustum_vertices[0], GL_DYNAMIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//pass indices to element array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFrustumIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frustum_indices), &frustum_indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
			 

	//setup a set of points
	for(int j=0;j<PZ;j++) {
		for(int i=0;i<PX;i++) {
			float   x = i/(PX-1.0f);
			float   z = j/(PZ-1.0f); 
			pointVertices[j*PX+i] = glm::vec3(x,0,z);
		}
	}
	//setup point vertex array and verex buffer objects
	glGenVertexArrays(1, &pointVAOID);
	glGenBuffers(1, &pointVBOID);
	glBindVertexArray(pointVAOID);
	glBindBuffer (GL_ARRAY_BUFFER, pointVBOID);
	//pass vertices to buffer object
	glBufferData (GL_ARRAY_BUFFER, sizeof(pointVertices), pointVertices, GL_STATIC_DRAW);

	//enable vertex attrib array for position
	glEnableVertexAttribArray(pointShader["vVertex"]);
	glVertexAttribPointer(pointShader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);

	// get the camera look direction to determine the yaw and pitch amount
	glm::vec3 look =  glm::normalize(cam.GetPosition());
	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;

	//if filtering is enabled, fill the mouse history buffer
	if(useFiltering) {
		for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
			mouseHistory[i] = glm::vec2(rX, rY);
		}
	}

	cout<<"Initialization successfull"<<endl;

	//get the initial time
	start_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
}

//delete all allocated objects
void OnShutdown() {
	glDeleteQueries(1, &query);

	//Destroy shader
	shader.DeleteShaderProgram();
	pointShader.DeleteShaderProgram();

	//Destroy vao and vbos
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	//Delete frustum vao and vbos
	glDeleteVertexArrays(1, &vaoFrustumID);
	glDeleteBuffers(1, &vboFrustumVerticesID);
	glDeleteBuffers(1, &vboFrustumIndicesID);

	//Delete point vao/vbo
	glDeleteVertexArrays(1, &pointVAOID);
	glDeleteBuffers(1, &pointVBOID);

	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

//idle callback function
void OnIdle() {

	//handle the WSAD, QZ key events to move the camera around
	if( GetAsyncKeyState(VK_W) & 0x8000) {
		pCurrentCam->Walk(dt);
	}

	if( GetAsyncKeyState(VK_S) & 0x8000) {
		pCurrentCam->Walk(-dt);
	}
	if( GetAsyncKeyState(VK_A) & 0x8000) {
		pCurrentCam->Strafe(-dt);
	}

	if( GetAsyncKeyState(VK_D) & 0x8000) {
		pCurrentCam->Strafe(dt);
	}

	if( GetAsyncKeyState(VK_Q) & 0x8000) {
		pCurrentCam->Lift(dt);
	}

	if( GetAsyncKeyState(VK_Z) & 0x8000) {
		pCurrentCam->Lift(-dt);
	}

	glm::vec3 t = pCurrentCam->GetTranslation(); 

	if(glm::dot(t,t)>EPSILON2) {
		pCurrentCam->SetTranslation(t*0.95f);
	}

	//update camera frustum
	if(pCurrentCam == &cam) {
		//get updated camera frustum vertices
		pCurrentCam->CalcFrustumPlanes();
		frustum_vertices[0] = cam.farPts[0];
		frustum_vertices[1] = cam.farPts[1];
		frustum_vertices[2] = cam.farPts[2];
		frustum_vertices[3] = cam.farPts[3];

		frustum_vertices[4] = cam.nearPts[0];
		frustum_vertices[5] = cam.nearPts[1];
		frustum_vertices[6] = cam.nearPts[2];
		frustum_vertices[7] = cam.nearPts[3];

		//update the frustum vertices on the GPU
		glBindVertexArray(vaoFrustumID);
			glBindBuffer (GL_ARRAY_BUFFER, vboFrustumVerticesID);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(frustum_vertices), &frustum_vertices[0]);
		glBindVertexArray(0);
	}

	//call the display function
	glutPostRedisplay();
}

//display callback function
void OnRender() {
	//FPS related calcualtion
	++total_frames;
	last_time = current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	dt = current_time-last_time;
	if( (current_time-last_time_fps) >1) {
		fps = total_frames/(current_time-last_time_fps);
		last_time_fps = current_time;
		total_frames = 0;
	}

	//clear color buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transformation
	glm::mat4 MV	= pCurrentCam->GetViewMatrix();
	glm::mat4 P     = pCurrentCam->GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

	//get the frustum planes
	glm::vec4 p[6];
	pCurrentCam->GetFrustumPlanes(p);

	//begin hardware query
	glBeginQuery(GL_PRIMITIVES_GENERATED, query);

	//bind point shader
	pointShader.Use();
		//set shader uniforms
		glUniform1f(pointShader("t"), current_time);
		glUniformMatrix4fv(pointShader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform4fv(pointShader("FrustumPlanes"), 6, glm::value_ptr(p[0]));

		//bind the point vertex array object
		glBindVertexArray(pointVAOID);
			//draw points
			glDrawArrays(GL_POINTS,0,MAX_POINTS);

	//unbind point shader
	pointShader.UnUse();

	//end hardware query
	glEndQuery(GL_PRIMITIVES_GENERATED);

	//check the query result to get the total number of visible points
	GLuint res;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &res);
	sprintf_s(buffer, "FPS: %3.3f :: Total visible points: %3d",fps, res);
	glutSetWindowTitle(buffer);

	//set the normal shader
	shader.Use();
	//bind the vetex array object for ground plane
	glBindVertexArray(vaoID);
		//set shader uniforms
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform4fv(shader("color"),1,white);
			//draw triangles
			glDrawElements(GL_TRIANGLES, TOTAL_INDICES, GL_UNSIGNED_SHORT, 0);
			
	//draw the local cam frustum when world cam is on
	if(pCurrentCam == &world) {
		//set shader uniforms
		glUniform4fv(shader("color"),1,red);

		//bind frustum vertex array object
	 	glBindVertexArray(vaoFrustumID);

		//enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//set polygon mode as fill
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//draw triangles
	 		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		//reset the line drawing mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//disable blending
		glDisable(GL_BLEND);
	}

	//unbind shader
	shader.UnUse();

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//Keyboard event handler to toggle the local and world camera 
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case '1':
			pCurrentCam = &cam;
		break;

		case '2':
			pCurrentCam = &world;
		break;
	}
	glutPostRedisplay();
}


int main(int argc, char** argv) {
	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Free Camera - OpenGL 3.3");

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
	glutKeyboardFunc(OnKey);
	glutIdleFunc(OnIdle);

	//call main loop
	glutMainLoop();

	return 0;
}