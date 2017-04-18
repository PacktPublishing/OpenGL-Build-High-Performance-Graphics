//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 2 - Simple OpenGL Primitives Drawing Demo (Grid with axes)
// Copyrights & Licenses:
//========================================================================

// GLFW library for creating OpenGL content and managing user inputs
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Window size
const int WINDOWS_WIDTH = 640*2;
const int WINDOWS_HEIGHT = 480;

//basic structure for a Vertex to simplify data passing
typedef struct
{
	GLfloat x, y, z;
	GLfloat r, g, b, a;
} Vertex;


//function prototype definitions
void drawLineSegment(Vertex v1, Vertex v2, GLfloat width=1.0f);
void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width);
void drawLineDemo();

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
 * Draw a line segment that connects two vertices
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
*  Demo code: Draws a grid and x, y axes 
*/
void drawLineDemo(){
	//draw a simple grid
	drawGrid(5.0f, 1.0f, 0.1f);

	//define the vertices and colors of the line segments
	Vertex v1 = {-5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f};
	Vertex v2 = {5.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f};
	Vertex v3 = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.7f};
	Vertex v4 = {0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.7f};

	//draw the line segments
	drawLineSegment(v1, v2, 10.0f);
	drawLineSegment(v3, v4, 10.0f);
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

		//Start demo
		drawLineDemo();	

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//end
