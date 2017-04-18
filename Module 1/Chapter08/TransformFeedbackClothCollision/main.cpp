
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "..\src\GLSLShader.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert> 


using namespace std;
#pragma comment(lib, "glew32.lib")

#define CHECK_GL_ERRORS assert(glGetError()==GL_NO_ERROR);

//screen size
const int width = 1024, height = 1024;

//total number of particles on X and Z axis
int numX = 21, numY=21;
const int total_points = (numX+1)*(numY+1);

//world space cloth size
int sizeX = 4,
	sizeY = 4;
float hsize = sizeX/2.0f;

//number of transform feedback iterations
const int NUM_ITER = 10;

//selected vertex index
int selected_index = -1;

//flag to display/hide masses
bool bDisplayMasses=true;

//spring struct for storing cloth springs
struct Spring {
	int p1, p2;			//indices of two vertices
	float rest_length;	//resting length
	float Ks, Kd;		//spring and damping constants
};

//cloth indices
vector<GLushort> indices;

//cloth springs
vector<Spring> springs;

vector<glm::vec4> X;		//cloth vertex current positions
vector<glm::vec4> X_last;	//cloth vertex previous positions
vector<glm::vec3> F;		//cloth vertex forces

//variables for camera transformation
int oldX=0, oldY=0;
float rX=10, rY=-45;
int state =1 ;
float dist=-4;

//grid size
const int GRID_SIZE=10;

//total number of springs
int spring_count=0;

//information message
char info[MAX_PATH]={0};

//default cloth spring values
const float DEFAULT_DAMPING =  -0.05f;
float	KsStruct = 10.5f,KdStruct = -10.5f;
float	KsShear = 0.25f,KdShear =-0.25f;
float	KsBend = 0.25f,KdBend = -0.25f;

//default gravity and mass 
glm::vec3 gravity=glm::vec3(0.0f,-0.00981f,0.0f);
float mass = 1.0f;

//default time step value and other timing related variables
float timeStep =  1.0f/60.0f;
float currentTime = 0;
double accumulator = timeStep;

//current viewport, modelview and projection matrices
//required for picking
GLint viewport[4];
GLdouble MV[16];
GLdouble P[16];

glm::mat4 mMVP;		//combined modelview projection matrix
glm::mat4 mMV;		//modelview matrix
glm::mat4 mP;		//projection matrix

//camera up, right and look vectors used in picking
glm::vec3 Up=glm::vec3(0,1,0), Right, viewDir;

//variables for high performance timer
LARGE_INTEGER frequency;        // ticks per second
LARGE_INTEGER t1, t2;           // ticks
double frameTimeQP=0;
float frameTime =0 ;
int texture_size_x=0;
int texture_size_y=0;

//for fps calculation
float startTime =0, fps=0 ;
int totalFrames=0;

//for transform feedback query
GLuint primitives_written=0;

//particle size
GLfloat pointSize = 30;

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
GLuint vaoUpdateID[2], vaoRenderID[2], vboIndices;

//buffer texture ids
GLuint texPosID[2];
GLuint texPrePosID[2];

size_t i=0;

//shaders for cloth vertex, particle and rendering
GLSLShader	massSpringShader,
			particleShader,
			renderShader;

//timer query ids
GLuint t_query, query;

//elapsed time per frame
GLuint64 elapsed_time;

//grid rendering variables
GLuint gridVAOID, gridVBOVerticesID, gridVBOIndicesID;
vector<glm::vec3> grid_vertices;
vector<GLushort> grid_indices;

//cloth vertex array and buffer objects
GLuint clothVAOID, clothVBOVerticesID, clothVBOIndicesID;

//spehre vertex array and buffer objects
GLuint sphereVAOID, sphereVerticesID, sphereIndicesID;

//transform feedback id
GLuint tfID;

//collision ellipsoid transform and its inverse 
glm::mat4 ellipsoid, inverse_ellipsoid;

//radius, stacks and slices for the sphere
int iStacks = 30;
int iSlices = 30;
float fRadius = 1;

//total sphere indices
int total_sphere_indices=0;

//converts vec4 to vec3
glm::vec3 vec3(glm::vec4 v) {
	return glm::vec3(v.x, v.y, v.z);
}

//creates a new spring between th given vertices (a and b) with the given
//spring and damping constants
void AddSpring(int a, int b, float ks, float kd ) {
	Spring spring;
	spring.p1=a;
	spring.p2=b;
	spring.Ks=ks;
	spring.Kd=kd;
	glm::vec3 deltaP = vec3(X[a]-X[b]);
	spring.rest_length = sqrt(glm::dot(deltaP, deltaP));
	springs.push_back(spring);
}

//creates buffer objects for cloth, grid and sphere
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
	glGenBuffers(1, &vboIndices);
	glGenTextures(2, texPosID);
	glGenTextures(2, texPrePosID);

	//set update vao
	for(int i=0;i<2;i++) {
		glBindVertexArray(vaoUpdateID[i]);
		//pass current positions 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Pos[i]);
		glBufferData( GL_ARRAY_BUFFER, X.size()* sizeof(glm::vec4), &(X[0].x), GL_DYNAMIC_COPY);
		//enable vertex attribute attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 0, 0);

		CHECK_GL_ERRORS

		//pass previous positions 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_PrePos[i]);
		glBufferData( GL_ARRAY_BUFFER, X_last.size()*sizeof(glm::vec4), &(X_last[0].x), GL_DYNAMIC_COPY);
		//enable vertex attribute attribute
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,  4, GL_FLOAT, GL_FALSE, 0,0);

		CHECK_GL_ERRORS;
	}

	CHECK_GL_ERRORS;

	//setup the render vao which simply uses the position buffer object we filled earlier
	for(int i=0;i<2;i++) {
		glBindVertexArray(vaoRenderID[i]);
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Pos[i]);
		//enable vertex attribute array
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 0, 0);

		//setup the indices array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
		if(i==0)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(0);

	//setup the two buffer textures, which take the values from the 
	//current and previous position buffer objects
	for(int i=0;i<2;i++) {
		glBindTexture( GL_TEXTURE_BUFFER, texPosID[i]);
		glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, vboID_Pos[i]);

		glBindTexture( GL_TEXTURE_BUFFER, texPrePosID[i]);
		glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, vboID_PrePos[i]);
	}
	 
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

	//create cloth vertex array and buffer objects
	glGenVertexArrays(1, &clothVAOID);
	glGenBuffers (1, &clothVBOVerticesID);
	glGenBuffers (1, &clothVBOIndicesID);
	glBindVertexArray(clothVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, clothVBOVerticesID);
		//pass cloth vertex positions 
		glBufferData (GL_ARRAY_BUFFER, sizeof(float)*4*X.size(), &X[0].x, GL_STATIC_DRAW);
		//enable vertex attribute array
		glEnableVertexAttribArray(0);
		glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE,0,0);

		CHECK_GL_ERRORS
		//pass cloth indices to element array buffer 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clothVBOIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*indices.size(), &indices[0], GL_STATIC_DRAW);

	//setup sphere vertices and indices
	vector<glm::vec4> sphere_vertices;
	vector<GLushort>  sphere_indices;

	//this code generates a set of vertices on a sphere
	//and also generate their triangulation

	GLfloat drho = (GLfloat)(M_PI) / (GLfloat) iStacks;
    GLfloat dtheta = 2.0f * (GLfloat)(M_PI) / (GLfloat) iSlices;
	GLfloat ds = 1.0f / (GLfloat) iSlices;
	GLfloat dt = 1.0f / (GLfloat) iStacks;
    GLint i, j;     // Looping variables
    total_sphere_indices = iSlices * iStacks * 6;
	count=0;
	 
	for (i = 0; i < iStacks; i++)
	{
		GLfloat rho = (GLfloat)i * drho;
		GLfloat srho = (GLfloat)(sin(rho));
		GLfloat crho = (GLfloat)(cos(rho));
		GLfloat srhodrho = (GLfloat)(sin(rho + drho));
		GLfloat crhodrho = (GLfloat)(cos(rho + drho));

        for ( j = 0; j < iSlices; j++)
		{
			GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;
			GLfloat stheta = (GLfloat)(-sin(theta));
			GLfloat ctheta = (GLfloat)(cos(theta));

			GLfloat x = stheta * srho;
			GLfloat y = ctheta * srho;
			GLfloat z = crho;
			//save sphere vertices
			sphere_vertices.push_back(glm::vec4(x,y,z,1) * fRadius);

            x = stheta * srhodrho;
			y = ctheta * srhodrho;
			z = crhodrho;
			//save sphere vertices
			sphere_vertices.push_back(glm::vec4(x,y,z,1) * fRadius);

			theta = ((j+1) == iSlices) ? 0.0f : (j+1) * dtheta;
			stheta = (GLfloat)(-sin(theta));
			ctheta = (GLfloat)(cos(theta));

			x = stheta * srho;
			y = ctheta * srho;
			z = crho;
			//save sphere vertices
			sphere_vertices.push_back(glm::vec4(x,y,z,1) * fRadius);

            x = stheta * srhodrho;
			y = ctheta * srhodrho;
			z = crhodrho;

			//save sphere vertices
			sphere_vertices.push_back(glm::vec4(x,y,z,1) * fRadius);

			//save sphere indices
			sphere_indices.push_back(count);
			sphere_indices.push_back(count+1);
			sphere_indices.push_back(count+2);
			sphere_indices.push_back(count+1);
			sphere_indices.push_back(count+3);
			sphere_indices.push_back(count+2);
			count+=4;
		}
	}


	//setup the sphere vertex array and buffer objects
	glGenVertexArrays(1, &sphereVAOID);
	glGenBuffers (1, &sphereVerticesID);
	glGenBuffers (1, &sphereIndicesID);
	glBindVertexArray(sphereVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, sphereVerticesID);
		//pass sphere vertices into buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(float)*4*sphere_vertices.size(), &sphere_vertices[0].x, GL_STATIC_DRAW);
		//enable vertex attribute array
		glEnableVertexAttribArray(0);
		glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE,0,0);

		CHECK_GL_ERRORS
		//pass sphere indices to element array object
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*total_sphere_indices, &sphere_indices[0], GL_STATIC_DRAW);


	glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
		//pass the current clicked position on screen
		//note that the y value is flipped
		int winY = (height - y); 
		int winX = x ; 
		float winZ =0;

		//read the depth value at the clicked position
		glReadPixels( winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
		if(winZ==1)
			winZ=0;

		//unproject the value using the current modelview, projection matrices and the current 
		//viewport to get the object space position of the clicked point
		double objX=0, objY=0, objZ=0;
		gluUnProject(winX, winY, winZ,  MV,  P, viewport, &objX, &objY, &objZ);
	
		glm::vec3 pt(objX,objY, objZ);
	 
		//bind our render vertex array object
		glBindVertexArray(vaoRenderID[readID]);
		glBindBuffer(GL_ARRAY_BUFFER, vboID_Pos[writeID]);
		//map the data so that we can read each vertex position
		//to check the nearest vertex the user has picked
		GLfloat* pData = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

		//loop through all cloth vertices
		for(int i=0;i<total_points*4;i+=4) {
			//if the vertex distance is close enough
			if( abs(pData[i]-pt.x)<0.1 &&
				abs(pData[i+1]-pt.y)<0.1  &&
				abs(pData[i+2]-pt.z)<0.1 ) {
				//take this as the selected vertex
				selected_index = i/4;
				printf("Intersected at %d\n",i);
				break;
			}
		}
		//unmap buffer to release the obtained pointer to GPU memory
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindVertexArray(0);

	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;

	if(s==GLUT_UP) {
		selected_index= -1;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void OnMouseMove(int x, int y)
{
	if(selected_index == -1) {
		if (state == 0)
			dist *= (1 + (y - oldY)/60.0f);
		else
		{
			rY += (x - oldX)/5.0f;
			rX += (y - oldY)/5.0f;
		}
	} else {

		//if there was a vertex selected, selected_index will not be -1
		float delta = 1500/abs(dist);
		float valX = (x - oldX)/delta;
		float valY = (oldY - y)/delta;
		//change the cursor depending on the direction the user is draggin mouse in
		if(abs(valX)>abs(valY))
			glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
		else
			glutSetCursor(GLUT_CURSOR_UP_DOWN);

		//set the rendering vertex array object
		glBindVertexArray(vaoRenderID[readID]);
		//bind the position buffer object
		glBindBuffer(GL_ARRAY_BUFFER, vboID_Pos[writeID]);
			//map the buffer object to obtain access to the selected vertex position
			GLfloat* pData = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
				//assign a new value to the selected vertex
				pData[selected_index*4]	  += Right[0]*valX ;
				float newValue = pData[selected_index*4+1]+Up[1]*valY;

				//for Y value, we test the new value to be >0 so that he vertex cannot go
				//below the ground plane
				if(newValue>0)
					pData[selected_index*4+1] = newValue;
				pData[selected_index*4+2] += Right[2]*valX + Up[2]*valY;
			//unmap buffer to release the obtained pointer to GPU memory
			glUnmapBuffer(GL_ARRAY_BUFFER);
		//do the same steps for the previous position buffer object
		glBindBuffer(GL_ARRAY_BUFFER, vboID_PrePos[writeID]);
			pData = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
				pData[selected_index*4]	  += Right[0]*valX ;
				newValue = pData[selected_index*4+1]+Up[1]*valY;
				if(newValue>0)
					pData[selected_index*4+1] = newValue;
				pData[selected_index*4+2] += Right[2]*valX + Up[2]*valY;
			glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindVertexArray(0);
	}

	oldX = x;
	oldY = y;

	glutPostRedisplay();
}


//grid rendering routine 
//it uses the pass through shader with the given uniform colour as fragment colour 
void DrawGrid()
{
	renderShader.Use();
		glBindVertexArray(gridVAOID);
		glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			glDrawElements(GL_LINES, grid_indices.size(),GL_UNSIGNED_SHORT,0);
		glBindVertexArray(0);
	renderShader.UnUse();
}

//renders cloth using the render shader 
void DrawCloth()
{
	renderShader.Use();
		glBindVertexArray(clothVAOID);
		glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			glDrawElements(GL_TRIANGLES, indices.size(),GL_UNSIGNED_SHORT,0);
		//glBindVertexArray(0);
	renderShader.UnUse();
}

//renders sphere using the render shader
void DrawSphere(glm::mat4 mvp) {
	renderShader.Use();
		glBindVertexArray(sphereVAOID);
		glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
			glDrawElements(GL_TRIANGLES, total_sphere_indices,GL_UNSIGNED_SHORT,0); 
	renderShader.UnUse();
}

//renders cloth vertices using particle shader
//this call assumes that the cloth vertex array object is bound currently
void DrawClothPoints()
{
	particleShader.Use();
		//glBindVertexArray(clothVAOID);
		glUniform1i(particleShader("selected_index"), selected_index);
		glUniformMatrix4fv(particleShader("MV"), 1, GL_FALSE, glm::value_ptr(mMV));
		glUniformMatrix4fv(particleShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			//draw the masses last
			glDrawArrays(GL_POINTS, 0, total_points);
		glBindVertexArray(0);
	particleShader.UnUse();
}

//initialize OpenGL
void InitGL() {
	//set the background colour to white
	glClearColor(1,1,1,1);

	//generate hardware query objects
	glGenQueries(1, &query);
	glGenQueries(1, &t_query);

	//setup texture size for shaders
	texture_size_x =  numX+1;
	texture_size_y =  numY+1;

	CHECK_GL_ERRORS
	
	//get initial time
	startTime = (float)glutGet(GLUT_ELAPSED_TIME);
	// get ticks per second
    QueryPerformanceFrequency(&frequency);

    // start timer
    QueryPerformanceCounter(&t1);

	//local variables
	size_t i=0, j=0, count=0;
	int l1=0, l2=0;
	int v = numY+1;
	int u = numX+1;

	printf("Total triangles: %3d\n",numX*numY*2);
	
	//resize the cloth indices, position, previous position and force vectors
	indices.resize( numX*numY*2*3);
	X.resize(total_points);
	X_last.resize(total_points);
	F.resize(total_points);

	//fill in positions
	for(int j=0;j<=numY;j++) {
		for(int i=0;i<=numX;i++) {
			X[count] = glm::vec4( ((float(i)/(u-1)) *2-1)* hsize, sizeX+1, ((float(j)/(v-1) )* sizeY),1);
			X_last[count] = X[count];
			count++;
		}
	}

	//fill in indices
	GLushort* id=&indices[0];
	for (int i = 0; i < numY; i++) {
		for (int j = 0; j < numX; j++) {
			int i0 = i * (numX+1) + j;
			int i1 = i0 + 1;
			int i2 = i0 + (numX+1);
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
	//set the polygon rendering to render them as lines
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//set smooth line rendering
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	 
	//enable state to use the vertex shader to reset the particle size
	//by writing to gl_PointSize value
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// Setup springs
	// Horizontal
	for (l1 = 0; l1 < v; l1++)	// v
		for (l2 = 0; l2 < (u - 1); l2++) {
			AddSpring((l1 * u) + l2,(l1 * u) + l2 + 1,KsStruct,KdStruct);
		}

	// Vertical
	for (l1 = 0; l1 < (u); l1++)
		for (l2 = 0; l2 < (v - 1); l2++) {
			AddSpring((l2 * u) + l1,((l2 + 1) * u) + l1,KsStruct,KdStruct);
		}

	// Shearing Springs
	for (l1 = 0; l1 < (v - 1); l1++)
		for (l2 = 0; l2 < (u - 1); l2++) {
			AddSpring((l1 * u) + l2,((l1 + 1) * u) + l2 + 1,KsShear,KdShear);
			AddSpring(((l1 + 1) * u) + l2,(l1 * u) + l2 + 1,KsShear,KdShear);
		}

	// Bend Springs
	for (l1 = 0; l1 < (v); l1++) {
		for (l2 = 0; l2 < (u - 2); l2++) {
			AddSpring((l1 * u) + l2,(l1 * u) + l2 + 2,KsBend,KdBend);
		}
		AddSpring((l1 * u) + (u - 3),(l1 * u) + (u - 1),KsBend,KdBend);
	}
	for (l1 = 0; l1 < (u); l1++) {
		for (l2 = 0; l2 < (v - 2); l2++) {
			AddSpring((l2 * u) + l1,((l2 + 2) * u) + l1,KsBend,KdBend);
		}
		AddSpring(((v - 3) * u) + l1,((v - 1) * u) + l1,KsBend,KdBend);
	}

	//setup shader loading
	massSpringShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/Spring.vert");
	particleShader.LoadFromFile(GL_VERTEX_SHADER,"shaders/Basic.vert");
	particleShader.LoadFromFile(GL_FRAGMENT_SHADER,"shaders/Basic.frag");
	renderShader.LoadFromFile(GL_VERTEX_SHADER,"shaders/Passthrough.vert");
	renderShader.LoadFromFile(GL_FRAGMENT_SHADER,"shaders/Passthrough.frag");
	
	//compile and link mass spring shader
	massSpringShader.CreateAndLinkProgram();
	massSpringShader.Use();
		//add attributes and uniforms
		massSpringShader.AddAttribute("position_mass");
		massSpringShader.AddAttribute("prev_position");
		massSpringShader.AddUniform("tex_position_mass");
		massSpringShader.AddUniform("tex_pre_position_mass");
		massSpringShader.AddUniform("MVP");
		massSpringShader.AddUniform("dt");
		massSpringShader.AddUniform("gravity");
		massSpringShader.AddUniform("ksStr");
		massSpringShader.AddUniform("ksShr");
		massSpringShader.AddUniform("ksBnd");
		massSpringShader.AddUniform("kdStr");
		massSpringShader.AddUniform("kdShr");
		massSpringShader.AddUniform("kdBnd");
		massSpringShader.AddUniform("DEFAULT_DAMPING");
		massSpringShader.AddUniform("texsize_x");
		massSpringShader.AddUniform("texsize_y");
		massSpringShader.AddUniform("step");
		massSpringShader.AddUniform("inv_cloth_size");
		massSpringShader.AddUniform("ellipsoid_xform");	
		massSpringShader.AddUniform("inv_ellipsoid");	
		massSpringShader.AddUniform("ellipsoid");		
	massSpringShader.UnUse();

	CHECK_GL_ERRORS

	//compile and link particle shader
	particleShader.CreateAndLinkProgram();
	particleShader.Use();
		//setup attributes and uniforms
		particleShader.AddAttribute("position_mass");
		particleShader.AddUniform("pointSize");
		particleShader.AddUniform("MV");
		particleShader.AddUniform("MVP");
		particleShader.AddUniform("vColor");
		particleShader.AddUniform("selected_index");
		//pass values to contant uniforms
		glUniform1f(particleShader("pointSize"), pointSize);
		glUniform4fv(particleShader("vColor"),1, vRed);
	particleShader.UnUse();

	//compile and link render shader
	renderShader.CreateAndLinkProgram();
	renderShader.Use();
		//setup attributes and uniforms
		renderShader.AddAttribute("position_mass");
		renderShader.AddUniform("MVP");
		renderShader.AddUniform("vColor");
		//pass values to constant uniforms
		glUniform4fv(renderShader("vColor"),1, vGray);
	renderShader.UnUse();

	CHECK_GL_ERRORS

	//create vbo
	createVBO();

	//collision ellipsoid
	ellipsoid = glm::translate(glm::mat4(1),glm::vec3(0,2,0));
	ellipsoid = glm::rotate(ellipsoid, 45.0f ,glm::vec3(1,0,0));
	ellipsoid = glm::scale(ellipsoid, glm::vec3(fRadius,fRadius,fRadius/2));
	inverse_ellipsoid = glm::inverse(ellipsoid);

	//setup transform feedback attributes
	glGenTransformFeedbacks(1, &tfID);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfID);
	//pass the vertex shader outputs for transform feedback
	const char* varying_names[]={"out_position_mass", "out_prev_position"};
	glTransformFeedbackVaryings(massSpringShader.GetProgram(), 2, varying_names, GL_SEPARATE_ATTRIBS);
	//relink the massSpringShader program
	glLinkProgram(massSpringShader.GetProgram());

	//set the mass spring shader and pass values to constant uniforms
	massSpringShader.Use();		
		glUniform1f(massSpringShader("dt"),  timeStep);
		glUniform3fv(massSpringShader("gravity"),1,&gravity.x);
		glUniform1i(massSpringShader("tex_position_mass"), 0);
		glUniform1i(massSpringShader("tex_pre_position_mass"), 1);
		glUniform1i(massSpringShader("texsize_x"),texture_size_x);
		glUniform1i(massSpringShader("texsize_y"),texture_size_y);
		glUniformMatrix4fv(massSpringShader("ellipsoid_xform"), 1, GL_FALSE, glm::value_ptr(ellipsoid));
		glUniformMatrix4fv(massSpringShader("inv_ellipsoid"), 1, GL_FALSE, glm::value_ptr(inverse_ellipsoid));
		glUniform4f(massSpringShader("ellipsoid"),0, 0, 0, fRadius);		
		glUniform2f(massSpringShader("inv_cloth_size"),float(sizeX)/numX,float(sizeY)/numY);
		glUniform2f(massSpringShader("step"),1.0f/(texture_size_x-1.0f),1.0f/(texture_size_y-1.0f));
		glUniform1f(massSpringShader("ksStr"),  KsStruct);
		glUniform1f(massSpringShader("ksShr"),  KsShear);
		glUniform1f(massSpringShader("ksBnd"),  KsBend);
		glUniform1f(massSpringShader("kdStr"),  KdStruct/1000.0f);
		glUniform1f(massSpringShader("kdShr"),  KdShear/1000.0f);
		glUniform1f(massSpringShader("kdBnd"),  KdBend/1000.0f);
		glUniform1f(massSpringShader("DEFAULT_DAMPING"),  DEFAULT_DAMPING);
	massSpringShader.UnUse();

	

	//disable vsync
	wglSwapIntervalEXT(0);
}
//resize event handler
void OnReshape(int nw, int nh) {
	//set the viewport
	glViewport(0,0,nw, nh);

	//set the projection matrix
	mP = glm::perspective(60.0f, (GLfloat)nw/nh, 1.0f, 100.f);
	for(int j=0;j<4;j++)
		for(int i=0;i<4;i++)
			P[i+j*4] = mP[j][i] ;

	//get the viewport
	glGetIntegerv(GL_VIEWPORT, viewport);
}
//update and rendering of cloth particles
void RenderGPU_TF() {
	CHECK_GL_ERRORS

	//set the cloth vertex shader
	massSpringShader.Use();
	CHECK_GL_ERRORS
		//pass shader uniforms
		glUniformMatrix4fv(massSpringShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
	 
		CHECK_GL_ERRORS
		//run the iteration loop
		for(int i=0;i<NUM_ITER;i++) {
			//set the buffer texture for current position 
			glActiveTexture( GL_TEXTURE0);
			glBindTexture( GL_TEXTURE_BUFFER, texPosID[writeID]);

			//set the buffer texture for previous position
			glActiveTexture( GL_TEXTURE1);
			glBindTexture( GL_TEXTURE_BUFFER, texPrePosID[writeID]);

			//set the update vertex array object
			glBindVertexArray( vaoUpdateID[writeID]);
				//bind transform feedback buffers
				//index 0 -> current position
				//index 1 -> previous position
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vboID_Pos[readID]);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vboID_PrePos[readID]);
				// disable rasterization
				glEnable(GL_RASTERIZER_DISCARD);  
					//run hardware timer query
					glBeginQuery(GL_TIME_ELAPSED,t_query);
						//initiate transform feedback
						glBeginTransformFeedback(GL_POINTS);
							//render points, this call pushes all attributes to GPU
							glDrawArrays(GL_POINTS, 0, total_points);
						//end transform feedback
						glEndTransformFeedback();
					//end timer query
					glEndQuery(GL_TIME_ELAPSED);
					glFlush();
				//enable rasterizer
				glDisable(GL_RASTERIZER_DISCARD);

			//swap read/write pathways
			int tmp = readID;
			readID=writeID;
			writeID = tmp;
		}
		CHECK_GL_ERRORS
		// get the query result
		glGetQueryObjectui64v(t_query, GL_QUERY_RESULT, &elapsed_time);
		//get the transform feedback time
		delta_time = elapsed_time / 1000000.0f;
	//remove the cloth vertex shader
	massSpringShader.UnUse();

	CHECK_GL_ERRORS;

	//bind the render vertex array object
	glBindVertexArray(vaoRenderID[writeID]);
		//disable depth test
		glDisable(GL_DEPTH_TEST);
			//set the render shader
			renderShader.Use();
				//set the shader uniform
				glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
					//draw the cloth geometry
					glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT,0);
			//remove render shader
			renderShader.UnUse();
		//enable depth test
		glEnable(GL_DEPTH_TEST);

		//if we want to display masses
		if(bDisplayMasses) {
			//set the particle shader
			particleShader.Use();
				//set shader uniforms
				glUniform1i(particleShader("selected_index"), selected_index);
				glUniformMatrix4fv(particleShader("MV"), 1, GL_FALSE, glm::value_ptr(mMV));
				glUniformMatrix4fv(particleShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
					//draw the masses last
			  		glDrawArrays(GL_POINTS, 0, total_points);
					//this also renders particles
					//glDrawTransformFeedbackStream(GL_POINTS, tfID, 0);
			//remove the particle shader
			particleShader.UnUse();
		}
	//remove the currently bound vertex array object
	glBindVertexArray( 0);

	CHECK_GL_ERRORS
}

//display function
void OnRender() {

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

	//FPS calculation code
	++totalFrames;
	if((newTime-startTime)>1000)
	{
		float elapsedTime = (newTime-startTime);
		fps = (totalFrames/ elapsedTime)*1000 ;
		startTime = newTime;
		totalFrames=0;
		sprintf_s(info, "FPS: %3.2f, Frame time (GLUT): %3.4f msecs, Frame time (QP): %3.3f, TF Time: %3.3f", fps, frameTime, frameTimeQP, delta_time);
	}

	glutSetWindowTitle(info);

	//clear colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	//set the camera transform
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -2.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	mMV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	mMVP = mP*mMV;

	for(int j=0;j<4;j++)
		for(int i=0;i<4;i++)
			MV[i+j*4] = mMV[j][i] ;

	//get the view and right vector
	viewDir.x = (float)-MV[2];
	viewDir.y = (float)-MV[6];
	viewDir.z = (float)-MV[10];
	Right = glm::cross(viewDir, Up);

	//draw grid
 	DrawGrid();

	//draw ellipsoids
	DrawSphere(mP*(mMV*ellipsoid));

	//deform and render cloth 
	RenderGPU_TF(); 

	//swap back and front buffers to display the result on screen
	glutSwapBuffers();
}

//delete all allocated objects
void OnShutdown() {
	X.clear();
	X_last.clear();
	F.clear();
	indices.clear();
	springs.clear();

	glDeleteQueries(1, &query);
	glDeleteQueries(1, &t_query);

	glDeleteTextures( 2, texPosID);
	glDeleteTextures( 2, texPrePosID);

	glDeleteVertexArrays(2, vaoUpdateID);
	glDeleteVertexArrays(2, vaoRenderID);
	glDeleteVertexArrays(1, &clothVAOID);
	glDeleteVertexArrays(1, &gridVAOID);
	glDeleteVertexArrays(1, &sphereVAOID);

	glDeleteBuffers( 1, &gridVBOVerticesID);
	glDeleteBuffers( 1, &gridVBOIndicesID);
	glDeleteBuffers( 1, &clothVBOVerticesID);
	glDeleteBuffers( 1, &clothVBOIndicesID);
	glDeleteBuffers( 1, &sphereVerticesID);
	glDeleteBuffers( 1, &sphereIndicesID);

    glDeleteBuffers( 2, vboID_Pos);
	glDeleteBuffers( 2, vboID_PrePos);
	glDeleteBuffers( 1, &vboIndices);

	glDeleteTransformFeedbacks(1, &tfID);
	renderShader.DeleteShaderProgram();
	massSpringShader.DeleteShaderProgram();
	particleShader.DeleteShaderProgram();
	printf("Shutdown successful");
}


void OnIdle() {
	glutPostRedisplay();
}
 
//keyboard event handler
void OnKey(unsigned char key, int , int) {
	switch(key) {
		case 'm':	bDisplayMasses=!bDisplayMasses;	break;
	}

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
	glutCreateWindow("GLUT Cloth Demo using Transform Feedback");

	//callback hooks
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnReshape);
	glutIdleFunc(OnIdle);

	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(OnKey);
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

