//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 6 - Rendering Stereoscopic 3D Models using OpenGL
// Copyrights & Licenses:
// ----------------------------------------------------------------------
// Keyboard shortcuts:
// ----------------------------------------------------------------------
// '1' - Mono, '2' - Stereo
// 'a' - rotate left, 's' - rotate right
// 'z' - rotate up, 'x' - rotate down
// 'up' - move forward, 'down' - move outward
// '3' - toggle draw line, '4' - toggle draw dots
// '5' - toggle draw surface, '6' - toggle depth test
//=======================================================================
#define GLM_FORCE_RADIANS

#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#ifdef _WIN32
#define GLEW_STATIC
#endif

//GLFW and GLEW libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <common.h>

#include <ObjLoader.hpp>

//Global variables
GLFWwindow* g_window;

const int WINDOWS_WIDTH = 1280;
const int WINDOWS_HEIGHT = 720;
float aspect_ratio = 16.0/9.0;
float z_offset = 0.0f;
float rotateY = 0.75f;
float rotateX = 1.0f;
float depthZ = 5.0f;
bool stereo = true;
bool drawLines = true;
bool drawPoints = true;
bool drawTriangles = true;
bool depthTest = false;

const char *hot_keys ="Hot Keys:"
"\n'1' - Mono, '2' - Stereo,\n'a' - rotate left, 's' - rotate right,"
"\n'z' - rotate up, 'x' - rotate down, 'up' - move forward, 'down' - move outward"
"\n'3' - toggle draw Line, '4' - toggle draw dots, '5' - toggle draw surface, '6' - toggle depth test";

//========================================================================
// Handle key strokes
//========================================================================
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;
	switch (key)
	{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_SPACE:
			rotateX=0;
			rotateY=0;
			break;
		case GLFW_KEY_Z:
			rotateX+=0.01;
			break;
		case GLFW_KEY_X:
			rotateX-=0.01;
			break;
		case GLFW_KEY_A:
			rotateY+=0.01;
			break;
		case GLFW_KEY_S:
			rotateY-=0.01;
			break;
		case GLFW_KEY_1:
			stereo = false;
			break;
		case GLFW_KEY_2:
			stereo = true;
			break;
		case GLFW_KEY_3:
			drawLines = !drawLines;
			break;
		case GLFW_KEY_4:
			drawPoints = !drawPoints;
			break;
		case GLFW_KEY_5:
			drawTriangles = !drawTriangles;
			break;
		case GLFW_KEY_UP:
			depthZ+=0.5f;
			break;
		case GLFW_KEY_DOWN:
			depthZ-=0.5f;
			break;
		case GLFW_KEY_6:
			depthTest = !depthTest;
			if(depthTest == true){
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
			}
			else{
				glDisable(GL_DEPTH_TEST);
				glEnable (GL_BLEND);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			break;
		default:
			break;
	}
}

int main(int argc, char **argv)
{
	//HOT Keys
	printf("%s\n", hot_keys);
	//Initialize GLFW
	if(!glfwInit()){
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(EXIT_FAILURE);
	}

	//enable anti-aliasing 4x with GLFW
	//glfwWindowHint(GLFW_SAMPLES, 4);
	//specify the client API version that the created context must be compatible with.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//make the GLFW forward compatible
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//use the OpenGL Core (http://www.opengl.org/wiki/Core_And_Compatibility_in_Contexts)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a GLFW window object
	g_window = glfwCreateWindow(WINDOWS_WIDTH, WINDOWS_HEIGHT, "Chapter 6 - Rendering 3D Models and Stereoscopic OpenGL", NULL, NULL);
	if(!g_window){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//make the context of the specified window current for the calling thread
	glfwMakeContextCurrent(g_window);
	glfwSwapInterval(1);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Final to Initialize GLEW\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//keyboard input callback
	glfwSetInputMode(g_window,GLFW_STICKY_KEYS,GL_TRUE);
	glfwSetKeyCallback(g_window, key_callback);

	GLuint program_id = LoadShaders("pointcloud.vert", "pointcloud.frag");
	ObjLoader *obj_loader = new ObjLoader();

	int result = 0;
	if(argc > 1){
		result = obj_loader->loadAsset(argv[1]);
	}else{
		result = obj_loader->loadAsset("obj_samples/dragon.obj");
		//with the custom orientation for optimal viewing
		rotateY = 0.75f;
		rotateX = 0.5f;
	}
	if(result){
		fprintf(stderr, "Final to Load the 3D file\n");
		glfwTerminate();
		exit(EXIT_FAILURE);

	}

	glDisable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//get the location for our "MVP" uniform variable
	GLuint matrix_id = glGetUniformLocation(program_id, "MVP");

	//use a large buffer to store the entire scene
	GLfloat	*g_vertex_buffer_data = (GLfloat*) malloc(obj_loader->getNumVertices()*sizeof(GLfloat));

	//load the scene into the vertex buffer
	obj_loader->loadVertices(g_vertex_buffer_data);

	//get the location for the attribute variables
	GLint attribute_vertex;

	attribute_vertex = glGetAttribLocation(program_id, "vertexPosition_modelspace");

	//generate the Vertex Array Object (Depedency: GLEW)
	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	//initialize the vertex buffer memory (similar to malloc)
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	//load data
	glBufferData(GL_ARRAY_BUFFER, obj_loader->getNumVertices()*sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW);

	//use our shader
	glUseProgram(program_id);

	//1st attribute buffer : vertices for position
	glEnableVertexAttribArray(attribute_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(attribute_vertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glPointSize(3.0f);

	do{
		float IPD = 0.65f;

		//clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int width, height;
		/*
		 * You are passing the window size, which is in screen coordinates,
		 * to glViewport, which works with pixels. On OS X with a Retina display,
		 * and possibly on other platforms in the future, screen coordinates and
		 * pixels do not map 1:1. Use the framebuffer size, which is in pixels,
		 * instead of the window size. See the Window handling guide for details.
		 */
		glfwGetFramebufferSize(g_window, &width, &height);

		if(stereo){
			//draw the left eye, left half of the screen
			glViewport(0, 0, width/2, height);

			//compute the MVP matrix from the IPD and virtual image plane distance
			computeStereoViewProjectionMatrices(g_window, IPD, depthZ, true);

			//get the View and Model Matrix and apply to the rendering
			glm::mat4 projection_matrix = getProjectionMatrix();
			glm::mat4 view_matrix = getViewMatrix();
			glm::mat4 model_matrix = glm::mat4(1.0);
			model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, -depthZ));

			model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
			model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateX, glm::vec3(1.0f, 0.0f, 0.0f));

			glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

			//send our transformation to the currently bound shader,
			//in the "MVP" uniform variable
			glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

			//render scene, with different drawing modes
			if(drawTriangles)
				obj_loader->draw(GL_TRIANGLES);
			if(drawPoints)
				obj_loader->draw(GL_POINTS);
			if(drawLines)
				obj_loader->draw(GL_LINES);

			//Draw the right eye, right half of the screen
			glViewport(width/2, 0, width/2, height);

			computeStereoViewProjectionMatrices(g_window, IPD, depthZ, false);
			projection_matrix = getProjectionMatrix();
			view_matrix = getViewMatrix();
			model_matrix = glm::mat4(1.0);
			model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, -depthZ));
			model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
			model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateX, glm::vec3(1.0f, 0.0f, 0.0f));

			mvp = projection_matrix * view_matrix * model_matrix;

			glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

			if(drawTriangles)
				obj_loader->draw(GL_TRIANGLES);
			if(drawPoints)
				obj_loader->draw(GL_POINTS);
			if(drawLines)
				obj_loader->draw(GL_LINES);
		}
		else{
			//draw the left eye (but full screen)
			glViewport(0, 0, width, height);

			//compute the MVP matrix from the IPD and virtual image plane distance
			computeStereoViewProjectionMatrices(g_window, IPD, depthZ, true);

			//get the View and Model Matrix and apply to the rendering
			glm::mat4 projection_matrix = getProjectionMatrix();
			glm::mat4 view_matrix = getViewMatrix();
			glm::mat4 model_matrix = glm::mat4(1.0);
			model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, -depthZ));

			model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
			model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateX, glm::vec3(1.0f, 0.0f, 0.0f));

			glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

			//send our transformation to the currently bound shader,
			//in the "MVP" uniform variable
			glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
			//render scene, with different mode, and can be enabled separately to get different effects
			if(drawTriangles)
				obj_loader->draw(GL_TRIANGLES);
			if(drawPoints)
				obj_loader->draw(GL_POINTS);
			if(drawLines)
				obj_loader->draw(GL_LINES);
		}
		//swap buffers
		glfwSwapBuffers(g_window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while(!glfwWindowShouldClose(g_window) && glfwGetKey(g_window, GLFW_KEY_ESCAPE )!=GLFW_PRESS);

	glDisableVertexAttribArray(attribute_vertex);
	//glDisableVertexAttribArray(attribute_uv);

	// Clean up VBO and shader
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteProgram(program_id);

	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(g_window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
