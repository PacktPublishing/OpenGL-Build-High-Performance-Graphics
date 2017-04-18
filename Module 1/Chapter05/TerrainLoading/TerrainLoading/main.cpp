#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

#include "..\..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//output screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shaders for use in the recipe
GLSLShader shader;

//IDs for vertex array and buffer object
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//heighmap texture ID
GLuint heightMapTextureID;

//heightmap texture dimensions and half dimensions
const int TERRAIN_WIDTH = 512;
const int TERRAIN_DEPTH = 512; 
const int TERRAIN_HALF_WIDTH = TERRAIN_WIDTH>>1;
const int TERRAIN_HALF_DEPTH = TERRAIN_DEPTH>>1;

//heightmap height scale and half scale values
float scale = 50;
float half_scale = scale/2.0f;

//total vertices and indices in the terrain
const int TOTAL = (TERRAIN_WIDTH*TERRAIN_DEPTH);
const int TOTAL_INDICES = TOTAL*2*3;

//heightmap filename
const string filename = "../../media/heightmap512x512.png";
 
//height map vertices and indices
glm::vec3 vertices[TOTAL];
GLuint indices[TOTAL_INDICES];

//projection and modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//camera transform variables
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -7;

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

//OpenGL initialization
void OnInit() {

	GL_CHECK_ERRORS
	//load heightmap shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();	
		//add attributes and uniforms
		shader.AddAttribute("vVertex"); 
		shader.AddUniform("heightMapTexture");
		shader.AddUniform("scale");
		shader.AddUniform("half_scale");
		shader.AddUniform("HALF_TERRAIN_SIZE");
		shader.AddUniform("MVP");
		//set values of constant uniforms as initialization	
		glUniform1i(shader("heightMapTexture"), 0);
		glUniform2i(shader("HALF_TERRAIN_SIZE"), TERRAIN_WIDTH>>1, TERRAIN_DEPTH>>1);
		glUniform1f(shader("scale"), scale);
		glUniform1f(shader("half_scale"), half_scale);
	shader.UnUse();

	GL_CHECK_ERRORS
		 
		
	//fill indices array
	GLuint* id=&indices[0];
	int i=0, j=0;
	
	//setup vertices 
	int count = 0;
	//fill terrain vertices
	for( j=0;j<TERRAIN_DEPTH;j++) {		 
		for( i=0;i<TERRAIN_WIDTH;i++) {	  
			vertices[count] = glm::vec3( (float(i)/(TERRAIN_WIDTH-1)), 
										 0, 
										 (float(j)/(TERRAIN_DEPTH-1)));
			count++;
		}
	}
	 
	//fill terrain indices
	for (i = 0; i < TERRAIN_DEPTH-1; i++) {        
		for (j = 0; j < TERRAIN_WIDTH-1; j++) {			
			int i0 = j+ i*TERRAIN_WIDTH;
			int i1 = i0+1;
			int i2 = i0+TERRAIN_WIDTH;
			int i3 = i2+1;
			*id++ = i0; 
			*id++ = i2; 
			*id++ = i1; 
			*id++ = i1; 
			*id++ = i2; 
			*id++ = i3; 
		}    
	}

	GL_CHECK_ERRORS

	//setup terrain vertex array and vertex buffer objects 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);
	 
	glBindVertexArray(vaoID);	

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//pass terrain vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS 
		//pass the terrain indices array to element array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
			
	//load the heightmap texture using SOIL	
	int texture_width = 0, texture_height = 0, channels=0;		 
	GLubyte* pData = SOIL_load_image(filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_L);
	
	//vertically flip the heightmap image on Y axis since it is inverted 
	for( j = 0; j*2 < texture_height; ++j )
	{
		int index1 = j * texture_width ;
		int index2 = (texture_height - 1 - j) * texture_width ;
		for( i = texture_width ; i > 0; --i )
		{
			GLubyte temp = pData[index1];
			pData[index1] = pData[index2];
			pData[index2] = temp;
			++index1;
			++index2;
		}
	} 

	//setup OpenGL texture
	glGenTextures(1, &heightMapTextureID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMapTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture_width, texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, pData);
	
	//free SOIL image data
	SOIL_free_image_data(pData);
	
	GL_CHECK_ERRORS

	//set polygon mode to draw lines
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GL_CHECK_ERRORS

	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {

	//Destroy shader
	shader.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	//Delete textures
	glDeleteTextures(1, &heightMapTextureID);
	cout<<"Shutdown successfull"<<endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	//setup the projection matrix
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.01f, 10000.f);
}

//display function
void OnRender() {
	//clear colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//set the camera transform
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 Ry	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 MV	= Ry;
    glm::mat4 MVP	= P*MV;

	//since we have kept the terrain vertex array object bound
	//it is still bound to the context so we can directly call draw element
	//which will draw vertices from the bound vertex array object
	//bind the terrain shader
	shader.Use();				
		//pass shader uniforms
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			//draw terrain mesh
			glDrawElements(GL_TRIANGLES, TOTAL_INDICES, GL_UNSIGNED_INT, 0);
	//unbind shader
	shader.UnUse();
	
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
	glutCreateWindow("Simple terrain - OpenGL 3.3");
	
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

	//call main loop
	glutMainLoop();	

	return 0;
}