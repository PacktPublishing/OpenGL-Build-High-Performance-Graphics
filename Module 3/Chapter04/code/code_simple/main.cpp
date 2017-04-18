//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 4 - Modern OpenGL - Texture Mapping with 2D images
// Copyrights & Licenses:
//=======================================================================

#include <stdio.h>
#include <stdlib.h>

//for Windows, we use the static version of GLEW
#ifdef _WIN32
#define GLEW_STATIC
#endif

//GLFW and GLEW libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <shader.hpp>

//Global variables
GLFWwindow* window;

int main(int argc, char **argv)
{
	//Initialize GLFW
	if(!glfwInit()){
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(EXIT_FAILURE);
	}

	//enable anti-aliasing 4x with GLFW
	glfwWindowHint(GLFW_SAMPLES, 4);
	//specify the client API version that the created context must be compatible with.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//make the GLFW forward compatible
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//use the OpenGL Core (http://www.opengl.org/wiki/Core_And_Compatibility_in_Contexts)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a GLFW windows object
	window = glfwCreateWindow(640, 480, "Chapter 4 - GLSL", NULL, NULL);
	if(!window){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	//make the context of the specified window current for the calling thread
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Final to Initialize GLEW\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

    GLuint program = LoadShaders("simple.vert", "simple.frag");

    glBindFragDataLocation(program, 0, "color_out");
    glUseProgram(program);

    // Create Vertex Array Object
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vertex_buffer;
    GLuint color_buffer;

    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &color_buffer);

    const GLfloat vertices[] = {
    		-1.0f, -1.0f, 0.0f,
    		1.0f, -1.0f, 0.0f,
    		1.0f, 1.0f, 0.0f,
    		-1.0f, -1.0f, 0.0f,
    		1.0f, 1.0f, 0.0f,
    		-1.0f, 1.0f, 0.0f
    };
    const GLfloat colors[]={
    		0.0f, 0.0f, 1.0f,
    		0.0f, 1.0f, 0.0f,
    		1.0f, 0.0f, 0.0f,
    		0.0f, 0.0f, 1.0f,
    		1.0f, 0.0f, 0.0f,
    		0.0f, 1.0f, 0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    // Specify the layout of the vertex data
    GLint position_attrib = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(position_attrib);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    GLint color_attrib = glGetAttribLocation(program, "color_in");
	glEnableVertexAttribArray(color_attrib);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glVertexAttribPointer(color_attrib,	3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	while(!glfwWindowShouldClose(window)){
		// Clear the screen to black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw a rectangle from the 2 triangles using 6 vertices
		glDrawArrays(GL_TRIANGLES, 0, 6); //draw the square

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//clear up the memories
    glDisableVertexAttribArray(position_attrib);
    glDisableVertexAttribArray(color_attrib);

    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &color_buffer);
    glDeleteVertexArrays(1, &vertex_array);

	glDeleteProgram(program);

	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

