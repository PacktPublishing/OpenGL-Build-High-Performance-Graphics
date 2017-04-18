//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 2 - 2-D Gaussian Demo
// Copyrights & Licenses:
//========================================================================

//GLFW library for creating OpenGL content and managing user inputs
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

#define _USE_MATH_DEFINES // M_PI constant 
#include <math.h>

// Window size
const int WINDOWS_WIDTH = 640;
const int WINDOWS_HEIGHT = 640;

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

void draw2DHeatMap(const Data *data, int num_points);
void guassianDemo(float sigma);

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
 * Draw a line segment
 */
void drawLineSegment(Vertex v1, Vertex v2, GLfloat width){
	glLineWidth(width);
	//line example
	glBegin(GL_LINES);
	glColor4f(v1.r, v1.g, v1.b, v1.a);
	glVertex3f(v1.x, v1.y, v1.z);
	glColor4f(v2.r, v2.g, v2.b, v2.a);
	glVertex3f(v2.x, v2.y, v2.z);
	glEnd();
}

/*
 * Visualize the data using a heat map 
 */
void draw2DHeatMap(const Data *data, int num_points){
	//locate the maximum and minimum values in the dataset
	float max_value=-999.9f;
	float min_value=999.9f;
	for(int i=0; i<num_points; i++){
		const Data d = data[i];
		if(d.z > max_value){
			max_value = d.z;
		}
		if(d.z < min_value){
			min_value = d.z;
		}
	}
	const float halfmax = (max_value + min_value) / 2;

	//display the result
	glPointSize(2.0f);
	glBegin(GL_POINTS);

	for(int i = 0; i<num_points; i++){
		const Data d = data[i];
		float value = d.z;
		float b = 1.0f - value/halfmax;
		float r = value/halfmax - 1.0f;
		if(b < 0){
			b=0;
		}
		if(r < 0){
			r=0;
		}
		float g = 1.0f - b - r;

		glColor4f(r, g, b, 0.5f);
		glVertex3f(d.x, d.y, 0.0f);
	}
	glEnd();
}

/*
 * Draw a 2-D Gaussian function using a heat map
 */
void gaussianDemo(float sigma){
	//construct a 1000x1000 grid 
	const int grid_x = 1000;
	const int grid_y = 1000;
	const int num_points = grid_x*grid_y;
	Data *data=(Data*)malloc(sizeof(Data)*num_points);
	int data_counter=0;
	for(int x = -grid_x/2; x<grid_x/2; x+=1){
		for(int y = -grid_y/2; y<grid_y/2; y+=1){
			float x_data = 2.0f*x/grid_x;
			float y_data = 2.0f*y/grid_y;
			//compute the height z based on a 2-D Gaussian function.
			float z_data = exp(-0.5f*(x_data*x_data)/(sigma*sigma) - 0.5f*(y_data*y_data)/(sigma*sigma))/(sigma*sigma*2.0f*M_PI);
			data[data_counter].x = x_data;
			data[data_counter].y = y_data;
			data[data_counter].z = z_data;
			data_counter++;
		}
	}
	//visualize the result using a 2D heat map
	draw2DHeatMap(data, num_points);
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

	float sigma = 0.01f;

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

		//simple grid
		drawGrid(5.0f, 1.0f, 0.1f);

		//draw the 2D Gaussian function with a heatmap
		sigma+=0.01f;
		if(sigma>1.0f)
			sigma=0.01;
		gaussianDemo(sigma);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//end
