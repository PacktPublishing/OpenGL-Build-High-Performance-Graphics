//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 5 - Rendering of Point Cloud Data for 3D Range Sensing Camera
// Copyrights & Licenses:
//=======================================================================

#define GLM_FORCE_RADIANS

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "controls.hpp"

//the view matrix and projection matrix
glm::mat4 g_view_matrix;
glm::mat4 g_projection_matrix;
glm::mat4 getViewMatrix(){
	return g_view_matrix;
}
glm::mat4 getProjectionMatrix(){
	return g_projection_matrix;
}


//initial position of the camera
glm::vec3 g_position = glm::vec3( 0, 0, 2 );
const float speed = 3.0f; // 3 units / second
float g_initial_fov = glm::pi<float>()*0.4f;

//compute the view matrix and projection matrix based on the
//user input
void computeViewProjectionMatrices(GLFWwindow* window){
	static double last_time = glfwGetTime();
	// Compute time difference between current and last frame
	double current_time = glfwGetTime();
	float delta_time = float(current_time - last_time);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	
	//direction vector for movement
	glm::vec3 direction(0, 0, -1);
	//up vector
	glm::vec3 up = glm::vec3(0,-1,0);

	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		g_position += direction * delta_time * speed;
	}
	else if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		g_position -= direction * delta_time * speed;
	}
	else if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
        g_initial_fov -= 0.1 * delta_time * speed;
	}
	else if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
        g_initial_fov += 0.1 * delta_time * speed;
	}
	// update projection matrix: Field of View, aspect ratio, display range : 0.1 unit <-> 100 units
	g_projection_matrix = glm::perspective(g_initial_fov, (float)width/(float)height, 0.1f, 100.0f);
	
	// update the view matrix
	g_view_matrix       = glm::lookAt(
			g_position,           // camera position
			g_position+direction, // viewing direction
			up                  // up direction
			);

	last_time = current_time;
}
