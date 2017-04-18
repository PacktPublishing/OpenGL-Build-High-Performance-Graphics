//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 2 - Simple OpenGL Primitives Drawing Demo (Sinusoid)
// Copyrights & Licenses:
//========================================================================

//GLFW library for creating OpenGL content and managing user inputs
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//window size
const int WINDOWS_WIDTH = 640*2;
const int WINDOWS_HEIGHT = 480;

//basic structure for a Vertex to simplify data passing
typedef struct
{
	GLfloat x, y, z;
	GLfloat r, g, b, a;
} Vertex;

//structure for data stream
typedef struct
{
	GLfloat x, y, z;
} Data;


//function prototype definitions
void drawPoint(Vertex v1, GLfloat size);
void drawPoints(const Vertex *array, GLfloat size, int num_points);
void drawLineSegment(Vertex v1, Vertex v2, GLfloat width=1.0f);
void drawLineSegments(Vertex *v_array, GLfloat width=1.0f);
void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width);
void draw2DScatterPlot(const Data *data, int num_points);
void draw2DLineSegments(const Data *data, int num_points);

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
 * Draw a 2D plot of some dataset
 * for simplicity
 */
void draw2DScatterPlot(const Data *data, int num_points){
	//draw a simple grid for the boundary
	Vertex v1 = {-10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
	Vertex v2 = {10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
	drawLineSegment(v1, v2, 2.0f);
	v1.x = 0.0f;
	v2.x = 0.0f;
	v1.y=-1.0f;
	v2.y=1.0f;
	drawLineSegment(v1, v2, 2.0f);

	//draw them as points
	for(int i=0; i<num_points; i++){
		//draw each of the data point in OpenGL
		GLfloat x=data[i].x;
		GLfloat y=data[i].y;
		Vertex v={x, y, 0.0f, 1.0, 1.0, 1.0, 0.7f};
		drawPoint(v, 10.0f);
	}
}

/**
 * Draw a 2D plot of some dataset, with connected lines
 */
void draw2DLineSegments(const Data *data, int num_points){
	//draw a vertical and horizontal line through the origin
	Vertex v1 = {-10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f};
	Vertex v2 = {10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f};
	drawLineSegment(v1, v2, 4.0f);
	v1.x = 0.0f;
	v2.x = 0.0f;
	v1.y=-1.0f;
	v2.y=1.0f;
	drawLineSegment(v1, v2, 4.0f);

	//draw them as points
	for(int i=0; i<num_points-1; i++){
		//draw each of the data point in OpenGL
		GLfloat x1=data[i].x;
		GLfloat y1=data[i].y;
		GLfloat x2=data[i+1].x;
		GLfloat y2=data[i+1].y;

		Vertex v1={x1, y1, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f};
		Vertex v2={x2, y2, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f};
		drawLineSegment(v1, v2, 4.0f);
	}
}

/**
 * Draw a single point
 */
void drawPoint(Vertex v1, GLfloat point_size){
	//draw a point and define the size, color, and location
	glPointSize(point_size);
	glBegin(GL_POINTS);
	glColor4f(v1.r, v1.g, v1.b, v1.a); // make this vertex red
	glVertex3f(v1.x, v1.y, v1.z);
	glEnd();
}

/**
 * Draw an array of points
 */
void drawPoints(const Vertex *v_array, GLfloat point_size, int num_points){
	//draw a point and define the size, color, and location
	glPointSize(point_size);
	glBegin(GL_POINTS);
	for(int i=0; i<num_points; i++){
		Vertex v1=v_array[i];
		glColor4f(v1.r, v1.g, v1.b, v1.a); // make this vertex red
		glVertex3f(v1.x, v1.y, v1.z);
	}
	glEnd();
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
 * Draw line segments for the array of vertices
 */
void drawLineSegments(Vertex *v_array, GLfloat width, int num_points){
	glLineWidth(width);
	//line example
	glBegin(GL_LINES);
	for(int i=0; i<num_points; i++){
		Vertex v1=v_array[i];
		glColor4f(v1.r, v1.g, v1.b, v1.a); // make this vertex red
		glVertex3f(v1.x, v1.y, v1.z);
	}
	glEnd();
}

/*
 * Demo for the line plots
 */
void linePlotDemo(float phase_shift){
	//simple grid
	drawGrid(5.0f, 1.0f, 0.1f);

	GLfloat range = 10.0f;
	const int num_points = 200;
	Data *data=(Data*)malloc(sizeof(Data)*num_points);
	for(int i=0; i<num_points; i++){
		data[i].x=((GLfloat)i/num_points)*range-range/2.0f;
		data[i].y= 0.8f*cosf(data[i].x*3.14f+phase_shift);
	}
	draw2DScatterPlot(data, num_points);
	draw2DLineSegments(data, num_points);
	free(data);
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

	float phase_shift=0.0f;

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

		//Cosine plot demo 
		phase_shift+=0.02f;
		linePlotDemo(phase_shift);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//end
