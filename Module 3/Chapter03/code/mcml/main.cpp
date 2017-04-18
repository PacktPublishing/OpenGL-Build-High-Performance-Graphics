//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 3 - 3D Volumetric Data Visualization
// Copyrights & Licenses:
// Instructions:
// - Left click and drag with the mouse to rotate around
// - Scroll up and down to zoom in and out (or Page Up/Down)
// - Press O or P to decrease or increase the size of each data point
// - Press Q or E to move the slice along the x axis (yz plane)
// - Press W or S to move the slice along the z axis (xy plane)
// - Press D or A to move the slice along the y axis (xz plane)
//========================================================================

//GLFW library for creating OpenGL content and managing user inputs
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Size of volumetric dataset in x, y, and z directions
#define MCML_SIZE_X 50
#define MCML_SIZE_Y 50
#define MCML_SIZE_Z 200

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

typedef struct {
	GLfloat r,g,b;
} Color;

// Window size
const int WINDOWS_WIDTH = 1280;
const int WINDOWS_HEIGHT = 720;

float mcml_data[MCML_SIZE_X][MCML_SIZE_Y][MCML_SIZE_Z];
Vertex mcml_vertices[MCML_SIZE_X][MCML_SIZE_Y][MCML_SIZE_Z];

float max_data, min_data;
int slice_x = 0, slice_z = 0, slice_y = 0;
float point_size=5.0f;

GLfloat alpha = 220.0f, beta = -60.0f, zoom = 2.5f;
GLboolean locked = GL_FALSE;

int cursorX = 0;
int cursorY = 0;

//========================================================================
// Jet scheme heat map rendering (advanced)
//========================================================================
Color getHeatMapColor(float v, float vmin, float vmax)
{
    //remapping the value to the JET heatmap
	Color c = {1.0f, 1.0f, 1.0f}; // white
	float dv;
	if (v < vmin)
		v = vmin;
	if (v > vmax)
		v = vmax;
	dv = vmax - vmin;

	if (v < (vmin + 0.25f * dv)) {
		c.r = 0.0f;
		c.g = 4.0f * (v - vmin) / dv;
	} else if (v < (vmin + 0.5f * dv)) {
		c.r = 0.0f;
		c.b = 1.0f + 4.0f * (vmin + 0.25f * dv - v) / dv;
	} else if (v < (vmin + 0.75f * dv)) {
		c.r = 4.0f * (v - vmin - 0.5f * dv) / dv;
		c.b = 0.0f;
	} else {
		c.g = 1.0f + 4.0f * (vmin + 0.75f * dv - v) / dv;
		c.b = 0.0f;
	}

	return(c);
}

//========================================================================
//Load the data from a text file and process them for heat map visualization
//(MCML_output.txt)
//========================================================================
void loadMCML(){
	FILE *ifp;

	//load volumetric dataset 
	ifp = fopen("MCML_output.txt", "r");
	if (ifp == NULL) {
		fprintf(stderr, "ERROR: Can't open MCML Data file!\n");
		exit(1);
	}

	float data;	
	float max=0, min=9999999;
	for(int x=0; x<MCML_SIZE_X; x++){
		for(int z=0; z<MCML_SIZE_Z; z++){
			for(int y=0; y<MCML_SIZE_Y; y++){

				if (fscanf(ifp, "%f\n", &data) == EOF){
					fprintf(stderr, "ERROR: Missing MCML Data file!\n");
					exit(1);
				}
				//store the data point
				data = log(data+1);
				mcml_data[x][y][z]=data;

				//find the max and min from the data set for heatmap
				if(data>max){
					max=data;
				}
				if(data<min){
					min=data;
				}

				//normalize the coordinates
				mcml_vertices[x][y][z].x=(float)(x-MCML_SIZE_X/2.0f)/MCML_SIZE_X;
				mcml_vertices[x][y][z].y=(float)(y-MCML_SIZE_Y/2.0f)/MCML_SIZE_Y;
				mcml_vertices[x][y][z].z=(float)(z-MCML_SIZE_Z/2.0f)/MCML_SIZE_Z*2.0f;
			}
		}
	}
	fclose(ifp);
	max_data = max;
	min_data = min;

	//store the heat map
	for(int z=0; z<MCML_SIZE_Z; z++){
		for(int x=0; x<MCML_SIZE_X; x++){
			for(int y=0; y<MCML_SIZE_Y; y++){
				float value = mcml_data[x][y][z];
				Color c = getHeatMapColor(value, min_data, max_data);
				mcml_vertices[x][y][z].r=c.r;
				mcml_vertices[x][y][z].g=c.g;
				mcml_vertices[x][y][z].b=c.b;
			}
		}
	}
}

//========================================================================
// Draw all MCML data points as point cloud
//========================================================================
void drawMCMLPoints(){
	glPointSize(point_size);

	glBegin(GL_POINTS);
	//display the result
	for(int z=0; z<MCML_SIZE_Z; z++){
		for(int x=0; x<MCML_SIZE_X; x++){
			for(int y=0; y<MCML_SIZE_Y; y++){
				glColor4f(mcml_vertices[x][y][z].r,mcml_vertices[x][y][z].g,mcml_vertices[x][y][z].b, 0.15f);
				glVertex3f(mcml_vertices[x][y][z].x, mcml_vertices[x][y][z].y, mcml_vertices[x][y][z].z);
			}
		}
	}
	glEnd();
}

//========================================================================
// Draw three orthogonal slices for 2D cross-sectional visualization
//========================================================================
void drawMCMLSlices(){
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	//display data on xy plane
	for(int x=0; x<MCML_SIZE_X; x++){
		for(int y=0; y<MCML_SIZE_Y; y++){
			int z = slice_z; 
			glColor4f(mcml_vertices[x][y][z].r,mcml_vertices[x][y][z].g,mcml_vertices[x][y][z].b, 0.9f);
			glVertex3f(mcml_vertices[x][y][z].x, mcml_vertices[x][y][z].y, mcml_vertices[x][y][z].z);
		}
	}
	//display data on yz plane
	for(int z=0; z<MCML_SIZE_Z; z++){
		for(int y=0; y<MCML_SIZE_Y; y++){
			int x = slice_x; 
			glColor4f(mcml_vertices[x][y][z].r,mcml_vertices[x][y][z].g,mcml_vertices[x][y][z].b, 0.9f);
			glVertex3f(mcml_vertices[x][y][z].x, mcml_vertices[x][y][z].y, mcml_vertices[x][y][z].z);
		}
	}
	//display data on xz plane
	for(int z=0; z<MCML_SIZE_Z; z++){
		for(int x=0; x<MCML_SIZE_X; x++){
			int y = slice_y; 
			glColor4f(mcml_vertices[x][y][z].r,mcml_vertices[x][y][z].g,mcml_vertices[x][y][z].b, 0.9f);
			glVertex3f(mcml_vertices[x][y][z].x, mcml_vertices[x][y][z].y, mcml_vertices[x][y][z].z);
		}
	}
	glEnd();
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
		case GLFW_KEY_P:
			point_size+=0.5;
			break;
		case GLFW_KEY_O:
			point_size-=0.5;
			break;
		case GLFW_KEY_A:
			slice_y -=1;
			if(slice_y < 0)
				slice_y = 0;
			break;
		case GLFW_KEY_D:
			slice_y +=1;
			if(slice_y >= MCML_SIZE_Y-1)
				slice_y = MCML_SIZE_Y-1;
			break;
		case GLFW_KEY_W:
			slice_z +=1;
			if(slice_z >= MCML_SIZE_Z-1)
				slice_z = MCML_SIZE_Z-1;
			break;
		case GLFW_KEY_S:
			slice_z -= 1;
			if (slice_z < 0)
				slice_z = 0;
			break;
		case GLFW_KEY_E:
			slice_x -=1;
			if(slice_x < 0)
				slice_x = 0;
			break;
		case GLFW_KEY_Q:
			slice_x +=1;
			if(slice_x >= MCML_SIZE_X-1)
				slice_x = MCML_SIZE_X-1;
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
	//if mouse button is pressed
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
	if (zoom < 0)
		zoom = 0;
}

//========================================================================
// Callback function for framebuffer resize events
//========================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	const double DEG2RAD = 3.14159265 / 180;
	const float fovY = 55.0f;
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

	//this will create the projection matrix based on the front
	//clipping plane, and the locations
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
	//draw a red line for x-axis
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

int main(int argc, char* argv[])
{
	loadMCML();

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
	//framebuffer size callback - i.e., resize the windows
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
	glEnable(GL_ALPHA_TEST);
	

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//draw the scene
		//switch to modelview so that the tranformation applies to entire model
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//move the object back and forth based on the zoom level
		glTranslatef(0.0, 0.0, -zoom);
		// rotate beta degrees around the x-axis
		glRotatef(beta, 1.0, 0.0, 0.0);
		// rotate alpha degrees around the z-axis
		glRotatef(alpha, 0.0, 0.0, 1.0);

	
		//disable depth test so we can render the points with blending
		glDisable(GL_DEPTH_TEST);
		drawMCMLPoints();

		//must enable this to ensure the slides are rendered in the right order 
		glEnable(GL_DEPTH_TEST);
		drawMCMLSlices();

		//draw the origin with the x,y,z axes for visualization
		drawOrigin();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//end
