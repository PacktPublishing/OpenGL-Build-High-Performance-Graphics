//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 5 - 3D Point Cloud Rendering
// Copyrights & Licenses:
//=======================================================================
#define GLM_FORCE_RADIANS

#ifdef _WIN32
#define GLEW_STATIC
#endif

#include <stdio.h>
#include <stdlib.h>

//GLFW and GLEW libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Common libraries
#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <common.h>

#include <fstream>

//Global variables
GLFWwindow* g_window;

const int WINDOWS_WIDTH = 1280;
const int WINDOWS_HEIGHT = 720;
const int IMAGE_WIDTH = 320;
const int IMAGE_HEIGHT = 240;
float aspect_ratio = 4.0 / 3.0;
float z_offset = 0.0f;
float rotateY = 0.0f;
float rotateX = 0.0f;

//Helper functions

//read the binary depth map image
unsigned short *readDepthFrame(const char *file_path){
	int depth_buffer_size = IMAGE_WIDTH*IMAGE_HEIGHT*sizeof(unsigned short);
	unsigned short *depth_frame = (unsigned short*)malloc(depth_buffer_size);
	ifstream myfile;
	myfile.open(file_path, ios::binary | ios::in);
	myfile.read((char*)depth_frame, depth_buffer_size);
	return depth_frame;
}
//read the binary color image
unsigned char *readColorFrame(const char *file_path){
	int color_buffer_size = IMAGE_WIDTH*IMAGE_HEIGHT*sizeof(unsigned char)* 3;
	unsigned char *color_frame = (unsigned char*)malloc(color_buffer_size);
	ifstream myfile;
	myfile.open(file_path, ios::binary | ios::in);
	myfile.read((char *)color_frame, color_buffer_size);
	return color_frame;
}

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
		rotateX = 0;
		rotateY = 0;
		break;
	case GLFW_KEY_Z:
		rotateX += 0.01;
		break;
	case GLFW_KEY_X:
		rotateX -= 0.01;
		break;
	case GLFW_KEY_A:
		rotateY += 0.01;
		break;
	case GLFW_KEY_S:
		rotateY -= 0.01;
		break;
	default:
		break;
	}
}

int main(int argc, char **argv)
{
	//Initialize GLFW
	if (!glfwInit()){
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	//enable anti-aliasing 4x with GLFW
	glfwWindowHint(GLFW_SAMPLES, 4);
	//specify the client API version that the created context must be compatible with.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//make GLFW forward compatible
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//use the OpenGL Core (http://www.opengl.org/wiki/Core_And_Compatibility_in_Contexts)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a GLFW windows object
	g_window = glfwCreateWindow(WINDOWS_WIDTH, WINDOWS_HEIGHT, "Chapter 5 - 3D Point Cloud Rendering", NULL, NULL);
	if (!g_window){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
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
	glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(g_window, key_callback);

	GLfloat	*g_vertex_buffer_data = (GLfloat*)malloc(IMAGE_WIDTH*IMAGE_HEIGHT * 3*sizeof(GLfloat));
	GLfloat *g_uv_buffer_data = (GLfloat*)malloc(IMAGE_WIDTH*IMAGE_HEIGHT * 2*sizeof(GLfloat));

	unsigned short *depth_frame = readDepthFrame("depth_frame0.bin");
	unsigned char *color_frame = readColorFrame("color_frame0.bin");

	//divided by two (320x240 instead of 640x480)
	float cx = 320.0f / 2.0f;
	float cy = 240.0f / 2.0f;
	float fx = 574.0f / 2.0f;
	float fy = 574.0f / 2.0f;
	for (int y = 0; y<IMAGE_HEIGHT; y++){
		for (int x = 0; x<IMAGE_WIDTH; x++){
			int index = y*IMAGE_WIDTH + x;
			float depth_value = (float)depth_frame[index] / 1000.0f; //in meter
			int ver_index = index * 3;
			int uv_index = index * 2;
			if (depth_value != 0){
				g_vertex_buffer_data[ver_index + 0] = ((float)x - cx)*depth_value / fx;
				g_vertex_buffer_data[ver_index + 1] = ((float)y - cy)*depth_value / fy;
				g_vertex_buffer_data[ver_index + 2] = -depth_value;

				g_uv_buffer_data[uv_index + 0] = (float)x / IMAGE_WIDTH;
				g_uv_buffer_data[uv_index + 1] = (float)y / IMAGE_HEIGHT;
			}
			else{
				g_vertex_buffer_data[ver_index + 0] = ((float)x - cx)*0.2f / fx;
				g_vertex_buffer_data[ver_index + 1] = ((float)y - cy)*0.2f / fy;
				g_vertex_buffer_data[ver_index + 2] = 0;
			}
		}
	}

	//now load all memory to OpenGL
	GLuint program_id = LoadShaders("pointcloud.vert", "pointcloud.frag");

	GLuint texture_id = loadRGBImageToTexture(color_frame, IMAGE_WIDTH, IMAGE_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	//get the location of the "MVP" uniform variable
	GLuint matrix_id = glGetUniformLocation(program_id, "MVP");

	//get the location of the "textureSampler" uniform variable
	GLuint texture_sampler_id = glGetUniformLocation(program_id, "textureSampler");

	//get the location of the attribute variables
	GLint attribute_vertex, attribute_uv;

	attribute_vertex = glGetAttribLocation(program_id, "vertexPosition_modelspace");
	attribute_uv = glGetAttribLocation(program_id, "vertexUV");

	//generate the Vertex Array Object (Depedency: GLEW)
	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	//initialize the vertex buffer memory (similar to malloc)
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, IMAGE_WIDTH*IMAGE_HEIGHT * 3 *sizeof(GLfloat) , g_vertex_buffer_data, GL_STATIC_DRAW);

	//initialize the UV buffer memory
	GLuint uv_buffer;
	//generate the array (like malloc)
	glGenBuffers(1, &uv_buffer);
	//bind the array buffer to that UV
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, IMAGE_WIDTH*IMAGE_HEIGHT * 2 * sizeof(GLfloat), g_uv_buffer_data, GL_STATIC_DRAW);

	//use our shader
	glUseProgram(program_id);

	//bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(texture_sampler_id, 0);

	//1st attribute buffer : vertices for position
	glEnableVertexAttribArray(attribute_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(attribute_vertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//2nd attribute buffer : UV mapping
	glEnableVertexAttribArray(attribute_uv);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	do{
		//clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

		//compute the MVP matrix from keyboard and mouse input
		computeViewProjectionMatrices(g_window);

		//get the View and Model Matrix and apply to the rendering
		glm::mat4 projection_matrix = getProjectionMatrix();
		glm::mat4 view_matrix = getViewMatrix();
		glm::mat4 model_matrix = glm::mat4(1.0);
		model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
		model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateX, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

		//send our transformation to the currently bound shader,
		//in the "MVP" uniform variable
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

		glPointSize(2.0f);

		//draw all points in space
		glDrawArrays(GL_POINTS, 0, IMAGE_WIDTH*IMAGE_HEIGHT);

		//swap buffers
		glfwSwapBuffers(g_window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(g_window) && glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS);

	glDisableVertexAttribArray(attribute_vertex);
	glDisableVertexAttribArray(attribute_uv);

	// Clean up VBO and shader
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &uv_buffer);
	glDeleteProgram(program_id);
	glDeleteTextures(1, &texture_id);
	glDeleteVertexArrays(1, &vertex_array_id);
	free(depth_frame);
	free(color_frame);
	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(g_window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}