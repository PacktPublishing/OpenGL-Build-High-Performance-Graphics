/*
 * AROverlayRenderer.h
 *
 */

#ifndef AROVERLAYRENDERER_H_
#define AROVERLAYRENDERER_H_

#include<Shader.hpp>

class AROverlayRenderer {
public:
	AROverlayRenderer();
	virtual ~AROverlayRenderer();
	void render();
	bool setup();
	void setScale(float s);

	void setOldRotMatrix(glm::mat4 r_matrix);
	void setRotMatrix(glm::mat4 r_matrix);
	void resetRotMatrix();
	void setScreenSize(int width, int height);
	void setDxDy (float dx, float dy);
private:
	//this renders the overlay view
	GLuint gProgramOverlay;
	GLuint gvOverlayPositionHandle;
	GLuint gvOverlayColorHandle;
	GLuint matrixHandle;
	GLuint sigmaHandle;
	GLuint scaleHandle;

	//vertices for the grid
	int grid_size;
	GLfloat *gGrid;
	GLfloat sigma;

	//for handling the object rotation from user
	GLfloat dx, dy;
	GLfloat rotX, rotY;

	//the view matrix and projection matrix
	glm::mat4 g_view_matrix;
	glm::mat4 g_projection_matrix;

	//initial position of the camera
	glm::vec3 g_position;
	//FOV of the virtual camera in OpenGL
	float g_initial_fov;

	glm::mat4 rotMatrix;
	glm::mat4 old_rotMatrix;

	float scale;
	int width;
	int height;

	Shader shader;
	void computeProjectionMatrices();
	void computeGrid();
};

#endif /* AROVERLAYRENDERER_H_ */
