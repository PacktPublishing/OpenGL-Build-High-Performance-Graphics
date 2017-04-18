//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 2 - Simple OpenGL Primitives Drawing Demo (Triangle)
// Copyrights & Licenses:
//========================================================================

//GLFW library for creating OpenGL content and managing user inputs
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//window size
const int WINDOWS_WIDTH = 640;
const int WINDOWS_HEIGHT = 480;

//basic structure for a Vertex to simplify data passing
typedef struct
{
	GLfloat x, y, z;
	GLfloat r, g, b, a;
} Vertex;

void drawLineSegment(Vertex v1, Vertex v2, GLfloat width=1.0f);
void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width);
void drawTriangleDemo();

/**
 * Draw a grid for visualization 
 */
void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width){
	//horizontal lines
	for(float i=-height; i<height; i+=grid_width){
		Vertex v1 = {-width, i, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
		Vertex v2 = {width, i, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
		drawLineSegment(v1, v2);
	}
	//vertical lines
	for(float i=-width; i<width; i+=grid_width){
		Vertex v1 = {i, -height, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
		Vertex v2 = {i, height, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
		drawLineSegment(v1, v2);
	}
}

/**
 * Draw a line segment
 */
void drawLineSegment(Vertex v1, Vertex v2, GLfloat width){
	glLineWidth(width);
	//line example
	glBegin(GL_LINES);
	glColor4f(v1.r, v1.g, v1.b, v1.a);
	glVertex3f(v1.x, v1.y, v1.z);
	glColor4f(v2.r, v2.g, v2.b, v2.a); // make this vertex red
	glVertex3f(v2.x, v2.y, v2.z);
	glEnd();
}

/**
 * Draw a triangle
 */
void drawTriangle(Vertex v1, Vertex v2, Vertex v3){
	//triangle example
	glBegin(GL_TRIANGLES);
	glColor4f(v1.r, v1.g, v1.b, v1.a);
	glVertex3f(v1.x, v1.y, v1.z);
	glColor4f(v2.r, v2.g, v2.b, v2.a);
	glVertex3f(v2.x, v2.y, v2.z);
	glColor4f(v3.r, v3.g, v3.b, v3.a);
	glVertex3f(v3.x, v3.y, v3.z);
	glEnd();
}

void drawTriangleDemo(){
	//Triangle Demo
	Vertex v1 = {0.0f, 0.8f, 0.0f, 1.0f, 0.0f, 0.0f, 0.6f};
	Vertex v2 = {-1.0f, -0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.6f};
	Vertex v3 = {1.0f, -0.8f, 0.0f, 0.0f, 0.0f, 1.0f, 0.6f};
	drawTriangle(v1, v2, v3);
}

int main(void)
{
	GLFWwindow* window;

	if (!glfwInit())
		exit(EXIT_FAILURE);
	window = glfwCreateWindow(WINDOWS_WIDTH, WINDOWS_HEIGHT, "Chapter 2: Primitive drawings", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	//enable anti-aliasing
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = (float) width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Orthographic Projection
		glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		glMatrixMode(GL_MODELVIEW);

		glLoadIdentity();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw a simple grid
		drawGrid(5.0f, 1.0f, 0.1f);
		
		//draw a triangle on top of the grid
		drawTriangleDemo();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//end
