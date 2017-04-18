//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 3 - 3D Surface Visualization
// Copyrights & Licenses:
// Instructions:
// - Left click and drag with the mouse to rotate around
//   (or up, down, left, right on keyboard)
// - Scroll up and down to zoom in and out (or Page Up/Down)
// - Press Space Bar to freeze the animation
//========================================================================

//GLFW library for creating OpenGL content and managing user inputs
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

#define _USE_MATH_DEFINES // M_PI constant 
#include <math.h>

// Window size
const int WINDOWS_WIDTH = 1280;
const int WINDOWS_HEIGHT = 720;

//freeze the animation
bool freeze = false;

//rotation angles and zoom factor for 3D rendering
GLfloat alpha=210.0f, beta=-70.0f, zoom=2.0f;
GLboolean locked = GL_FALSE;

int cursorX = 0;
int cursorY = 0;

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

//========================================================================
// Simplified heat map rendering
//========================================================================
void draw2DHeatMap(const Data *data, int num_points){
	glPointSize(3.0f);
	glBegin(GL_POINTS);
	float transparency = 0.25f; 

	//locate the maximum and minimum values in the dataset
	float max_value=-999.9f;
	float min_value=999.9f;
	for(int i=0; i<num_points; i++){
		Data d = data[i];
		if(d.z > max_value)
			max_value = d.z;
		if(d.z < min_value)
			min_value = d.z;
	}
	float halfmax = (max_value + min_value) / 2;

	//display the result
	for(int i = 0; i<num_points; i++){
		Data d = data[i];
		float value = d.z;
		float b = 1.0f - value/halfmax;
		float r = value/halfmax - 1.0f;
		if(b < 0)
			b=0;
		if(r < 0)
			r=0;
		float g = 1.0f - b - r;

		glColor4f(r, g, b, transparency);
		glVertex3f(d.x, d.y, d.z);
	}
	glEnd();
}

//========================================================================
// Draw a 3D grid with the Gaussian distribution
//========================================================================
void gaussianDemo(float sigma){
	//construct a 400x400 grid 
	const int grid_x = 400;
	const int grid_y = 400;
	const int num_points = grid_x*grid_y;
	Data *data=(Data*)malloc(sizeof(Data)*num_points);
	int data_counter=0;

	//standard deviation
	const float sigma2=sigma*sigma;
	
	//amplitude
	const float sigma_const = 10.0f*(sigma2*2.0f*(float)M_PI);

	for(float x = -grid_x/2.0f; x<grid_x/2.0f; x+=1.0f){
		for(float y = -grid_y/2.0f; y<grid_y/2.0f; y+=1.0f){
			float x_data = 2.0f*x/grid_x;
			float y_data = 2.0f*y/grid_y;
			//let the mean be 0
			float z_data = exp(-0.5f*(x_data*x_data)/(sigma2) -
					0.5f*(y_data*y_data)/(sigma2))/sigma_const;
			data[data_counter].x = x_data;
			data[data_counter].y = y_data;
			data[data_counter].z = z_data;
			data_counter++;
		}
	}
 	draw2DHeatMap(data, num_points);
	free(data);
}


//========================================================================
// Handle key strokes
//========================================================================
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	switch (key)
	{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_SPACE:
			freeze=!freeze;
			break;
		case GLFW_KEY_LEFT:
			alpha += 5.0f;
			break;
		case GLFW_KEY_RIGHT:
			alpha -= 5.0f;
			break;
		case GLFW_KEY_UP:
			beta -= 5.0f;
			break;
		case GLFW_KEY_DOWN:
			beta += 5.0f;
			break;
		case GLFW_KEY_PAGE_UP:
			zoom -= 0.25f;
			if (zoom < 0.0f)
				zoom = 0.0f;
			break;
		case GLFW_KEY_PAGE_DOWN:
			zoom += 0.25f;
			break;
		default:
			break;
	}
}


//========================================================================
// Callback function for mouse button events
//========================================================================
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		locked = GL_TRUE;
	}
	else
	{
		locked = GL_FALSE;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

//========================================================================
// Callback function for cursor motion events
//========================================================================
void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	//if the mouse button is pressed
	if (locked)
	{
		alpha += (GLfloat) (x - cursorX) / 10.0f;
		beta += (GLfloat) (y - cursorY) / 10.0f;
	}
	//update the cursor position
	cursorX = (int) x;
	cursorY = (int) y;
}

//========================================================================
// Callback function for scroll events
//========================================================================
void scroll_callback(GLFWwindow* window, double x, double y)
{
	zoom += (float) y / 4.0f;
	if (zoom < 0.0f)
		zoom = 0.0f;
}

//========================================================================
// Callback function for framebuffer resize events
//========================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	const double DEG2RAD = 3.14159265 / 180;
	const float fovY = 45.0f;
	const float front = 0.1f;
	const float back = 128.0f;
	float ratio = 1.0f;
	if (height > 0)
		ratio = (float) width / (float) height;

	// Setup viewport
	glViewport(0, 0, width, height);

	// Change to the projection matrix and set our viewing volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double tangent = tan(fovY/2 * DEG2RAD);   // tangent of half fovY
	double height_f = front * tangent;          // half height of near plane
	double width_f = height_f * ratio;      // half width of near plane

	//Create the projection matrix based on the front
	//clipping plane and the location of the corners
	glFrustum(-width_f, width_f, -height_f, height_f, front, back);

	//Alternative: gluPerspective will provide the same solution as above
	//gluPerspective(fovY, ratio, front, back);
}

//========================================================================
// Draw the x,y,z axes at the origin
//========================================================================
void drawOrigin(){
	float transparency = 0.5f;
	glLineWidth(4.0f);
	glBegin(GL_LINES);
	//draw a red line for the x-axis
	glColor4f(1.0f, 0.0f, 0.0f, transparency);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glColor4f(1.0f, 0.0f, 0.0f, transparency);
	glVertex3f(0.3f, 0.0f, 0.0f);

	//draw a green line for the y-axis
	glColor4f(0.0f, 1.0f, 0.0f, transparency);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glColor4f(0.0f, 1.0f, 0.0f, transparency);
	glVertex3f(0.0f, 0.0f, 0.3f);

	//draw a blue line for the z-axis
	glColor4f(0.0f, 0.0f, 1.0f, transparency);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glColor4f(0.0f, 0.0f, 1.0f, transparency);
	glVertex3f(0.0f, 0.3f, 0.0f);
	glEnd();
}

int main(void)
{
	GLFWwindow* window;
	int width, height;

	if (!glfwInit()){
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(WINDOWS_WIDTH, WINDOWS_HEIGHT, "Chapter 3: 3D Data Plotting", NULL, NULL);
	if (!window){
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//initialize the callbacks for event handling
	//keyboard input callback
	glfwSetKeyCallback(window, key_callback);
	//framebuffer size callback - i.e., window resizing
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//mouse button callback
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	//mouse movement callback
	glfwSetCursorPosCallback(window, cursor_position_callback);
	//mouse scroll callback
	glfwSetScrollCallback(window, scroll_callback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	//get the frame buffer (window) size
	glfwGetFramebufferSize(window, &width, &height);
	//initial call to the framebuffer callback, and initialize the OpenGL
	//camera and other properties there
	framebuffer_size_callback(window, width, height);


	//enable anti-aliasing
	glEnable(GL_BLEND);
	//smooth the points
	glEnable(GL_LINE_SMOOTH);
	//smooth the lines
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	//needed for alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST) ;

	//initialize all parameters
	float sigma = 0.1f;
	float sign = 1.0f;
	float step_size = 0.01f;

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//draw the scene
		//switch to modelview so the tranformation applies to entire model
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//move the object back and forth based on the zoom level
		glTranslatef(0.0, 0.0, -zoom);
		// rotate beta degrees around the x-axis
		glRotatef(beta, 1.0, 0.0, 0.0);
		// rotate alpha degrees around the z-axis
		glRotatef(alpha, 0.0, 0.0, 1.0);

		//draw the origin with the x,y,z axes for visualization
		drawOrigin();

		//draw the gaussian distribution in 2D with heatmap
		if(!freeze){
			sigma=sigma+sign*step_size;
			if(sigma>1.0f){
				sign = -1.0f;
			}
			if(sigma<0.1){
				sign = 1.0f;
			}
		}
		gaussianDemo(sigma);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//end
