//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 2 - ECG time series demo
// Copyrights & Licenses:
//========================================================================

//GLFW library for creating OpenGL content and managing user inputs
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include "data_ecg.h"

//Window size
const int WINDOWS_WIDTH = 640*2;
const int WINDOWS_HEIGHT = 480;

#define ECG_DATA_BUFFER_SIZE  1024
float ratio;

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
void drawLineSegment(Vertex v1, Vertex v2, GLfloat width=1.0f);
void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width);
void plotECGData(int offset, int size, float offset_y, float scale);
void ecg_demo(int counter);

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
 * Draw a line segment on screen
 */
void drawLineSegment(Vertex v1, Vertex v2, GLfloat width){
	glLineWidth(width);
	glBegin(GL_LINES);
	glColor4f(v1.r, v1.g, v1.b, v1.a);
	glVertex3f(v1.x, v1.y, v1.z);
	glColor4f(v2.r, v2.g, v2.b, v2.a); 
	glVertex3f(v2.x, v2.y, v2.z);
	glEnd();
}

/*
 * Plot the data from the data_ecg dataset.
 */
void plotECGData(int offset, int size, float offset_y, float scale){
	//space between samples
	const float space = 2.0f/size*ratio; 
	
	//initial position of the first vertex to render
	float pos = -size*space/2.0f;
    
	//set the width of the line
	glLineWidth(5.0f);
	
	glBegin(GL_LINE_STRIP);
    
	//set the color of the line to green
	glColor4f(0.1f, 1.0f, 0.1f, 0.8f);
	
	for (int i=offset; i<size+offset; i++){
		const float data = scale*data_ecg[i]+offset_y;
		glVertex3f(pos, data, 0.0f);
		pos += space;
	}
	glEnd();
}

/*
 * Helper function for running ECG demo
 */
void ecg_demo(int counter){
    const int data_size=ECG_DATA_BUFFER_SIZE;
    
    //Emulate the presence of multiple ECG leads (just for demo/display purposes)
    plotECGData(counter, data_size, -0.5f, 0.1f);
    plotECGData(counter+data_size, data_size, 0.0f, 0.5f);
    plotECGData(counter+data_size*2, data_size, 0.5f, -0.25f);
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

	int counter = 0;
	while (!glfwWindowShouldClose(window))
	{
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
        
		//aspect ratio of the screen (changes upon resizing the window)
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

		//simple grid
		drawGrid(5.0f, 1.0f, 0.1f);
		
		//reset counter to 0 after reaching the end of the sample data
		if(counter>5000){
			counter=0;
		}
		counter+=5;

		//run the demo visualizer
		ecg_demo(counter);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//end
