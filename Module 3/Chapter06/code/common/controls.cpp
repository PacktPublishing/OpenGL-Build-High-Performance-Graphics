//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 6 - Rendering Stereoscopic 3D Models using OpenGL
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
glm::vec3 g_position = glm::vec3( 0, 0, 0.0 );
const float speed = 3.0f; // 3 units / second
float g_initial_fov = glm::pi<float>()*0.25f;

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
	glm::vec3 direction_z(0, 0, -0.5);
	glm::vec3 direction_y(0, 0.5, 0);
	glm::vec3 direction_x(0.5, 0, 0);

	//up vector
	glm::vec3 up = glm::vec3(0,-1,0);

	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		g_position += direction_y * delta_time * speed;
	}
	else if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		g_position -= direction_y * delta_time * speed;
	}
	else if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		g_position += direction_z * delta_time * speed;
	}
	else if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		g_position -= direction_z * delta_time * speed;
	}
	else if (glfwGetKey( window, GLFW_KEY_PERIOD ) == GLFW_PRESS){
		g_position -= direction_x * delta_time * speed;
	}
	else if (glfwGetKey( window, GLFW_KEY_COMMA ) == GLFW_PRESS){
		g_position += direction_x * delta_time * speed;
	}


	float aspect_ratio = (float)width/(float)height;
	float nearZ = 0.1f;
	float farZ = 100.0f;
	float top = tan(g_initial_fov/2*nearZ);
	float right = aspect_ratio*top;
	float left = -right;
	float bottom = -top;

	g_projection_matrix = glm::frustum(left, right, bottom, top, nearZ, farZ);
	
	// update the view matrix
	g_view_matrix       = glm::lookAt(
			g_position,           // camera position
			g_position+direction_z, // viewing direction
			up                  // up direction
			);

	last_time = current_time;
}

void computeStereoViewProjectionMatrices(GLFWwindow* window, float IPD, float depthZ, bool left_eye){
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	//up vector
	glm::vec3 up = glm::vec3(0,-1,0);
	glm::vec3 direction_z(0, 0, -1);

	float left_right_direction = -1.0f;
	if(left_eye)
		left_right_direction = 1.0f; //left eye

	float aspect_ratio = (float)width/(float)height;
	float nearZ = 1.0f;
	float farZ = 100.0f;
    double frustumshift = (IPD/2)*nearZ/depthZ;
	float top = tan(g_initial_fov/2)*nearZ;
	float right = aspect_ratio*top+frustumshift*left_right_direction; //half screen
	float left = -aspect_ratio*top+frustumshift*left_right_direction;
	float bottom = -top;

	g_projection_matrix = glm::frustum(left, right, bottom, top, nearZ, farZ);

	// update the view matrix
	g_view_matrix       = glm::lookAt(
			g_position-direction_z+glm::vec3(left_right_direction*IPD/2, 0, 0), // eye position
			g_position+glm::vec3(left_right_direction*IPD/2, 0, 0), // centre position
			up// up direction
			);

}

