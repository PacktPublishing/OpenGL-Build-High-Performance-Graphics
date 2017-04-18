//========================================================================
// Authors: Raymond Lo and William Lo
// Simple GLFW example for getting started in OpenGL 
// Copyrights & Licenses:
//========================================================================

//Include the GLFW library for creating windows with OpenGL contexts and managing user inputs
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	//Initialize GLFW and create a GLFW window object (640 x 480).
	if (!glfwInit())
		exit(EXIT_FAILURE);

	GLFWwindow* window;
	window = glfwCreateWindow(640, 480, "Chapter 1: Simple GLFW Example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//make sure that the context of the specified window is current on the calling thread
	glfwMakeContextCurrent(window);

	//Define a loop which terminates when the window is closed.
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		
		//get the current width and height of the window,
		//in case the window is resized by the user
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		//Set up the viewport (using the width and height of the window) 
		//and clear the screen color buffer.
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		//Set up the camera matrix. Note: further details on the camera model will be discussed in Chapter 3
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Draw a rotating triangle and set a different color (red, green, and blue channels) for each vertex (x, y, z) of the triangle.
		glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f); //Rotate the triangle over time.

		glBegin(GL_TRIANGLES);
		glColor3f(1.f, 0.f, 0.f);
		glVertex3f(-0.6f, -0.4f, 0.f);
		glColor3f(0.f, 1.f, 0.f);
		glVertex3f(0.6f, -0.4f, 0.f);
		glColor3f(0.f, 0.f, 1.f);
		glVertex3f(0.f, 0.6f, 0.f);
		glEnd();

		//Swap the draw buffer
		glfwSwapBuffers(window);
		
		//for handling events such as keyboard input
		glfwPollEvents();
	}

	//Release the memory and terminate the GLFW library. Then, exit the application.
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
