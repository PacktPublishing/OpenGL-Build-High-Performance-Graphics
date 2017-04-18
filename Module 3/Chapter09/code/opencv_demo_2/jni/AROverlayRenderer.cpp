/*
 * AROverlayRenderer.cpp
 * This is the overlay function for augmented reality. The camera is setup such that
 * the intrinsic parameters are matched with the mobile camera. As long as the focal length
 * is fixed, we can reuse the same parameter for the same camera everytime.
 *
 * For more calibration related information, see
 * http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html
 *
 */

#include "AROverlayRenderer.hpp"

#define  LOG_TAG    "AROverlayRenderer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


AROverlayRenderer::AROverlayRenderer() {
	//initial position of the camera
	g_position = glm::vec3( 0.0f, 0.0f, 0.0f );

	//FOV of the virtual camera in OpenGL
	//Estimate or extract this parameter from the camera manufacturers' spec
	//45 degree FOV
	g_initial_fov = 45.0f*glm::pi<float>()/180.0f;

	//scale for the panel and other objects, allow for zooming in with pinch.
	scale = 1.0f;
	dx=0.0f; dy=0.0f;
	rotX=0.0f, rotY=0.0f;
	sigma = 0;

	grid_size = 400;
	//allocate memory for the grid
	gGrid = (GLfloat*) malloc(sizeof(GLfloat)*grid_size*grid_size*3);
}

AROverlayRenderer::~AROverlayRenderer() {
	//delete all dynamically allocated objects here
	free(gGrid);
}

/**
 * Initialize the grid pattern vertices
 */
void AROverlayRenderer::computeGrid(){
	float grid_x = grid_size;
	float grid_y = grid_size;
	unsigned int data_counter = 0;
	//define a grid ranging from -1 to +1
	for(float x = -grid_x/2.0f; x<grid_x/2.0f; x+=1.0f){
		for(float y = -grid_y/2.0f; y<grid_y/2.0f; y+=1.0f){
			float x_data = x/grid_x;
			float y_data = y/grid_y;
			gGrid[data_counter] = x_data;
			gGrid[data_counter+1] = y_data;
			gGrid[data_counter+2] = 0;
			data_counter+=3;
		}
	}
}
/**
 * Setup the Shader program for overlay graphics
 */
bool AROverlayRenderer::setup(){
	// Vertex shader source code
	static const char g_vshader_code_overlay[] =
		"#version 300 es\n"
	    "in vec4 vPosition;\n"
	    "uniform mat4 MVP;\n"
		"uniform float sigma;\n"
		"uniform float scale;\n"
	    "out vec4 color_based_on_position;\n"
	    "// Heat map generator                \n"
	    "vec4 heatMap(float v, float vmin, float vmax){\n"
	    "    float dv;\n"
	    "    float r=1.0, g=1.0, b=1.0;\n"
	    "	if (v < vmin){\n"
	    "		v = vmin;}\n"
	    "	if (v > vmax){\n"
	    "		v = vmax;}\n"
	    "	dv = vmax - vmin;\n"
	    "	if (v < (vmin + 0.25 * dv)) {\n"
	    "		r = 0.0;\n"
	    "		g = 4.0 * (v - vmin) / dv;\n"
	    "	} else if (v < (vmin + 0.5 * dv)) {\n"
	    "		r = 0.0;\n"
	    "		b = 1.0 + 4.0 * (vmin + 0.25 * dv - v) / dv;\n"
	    "	} else if (v < (vmin + 0.75 * dv)) {\n"
	    "		r = 4.0 * (v - vmin - 0.5 * dv) / dv;\n"
	    "		b = 0.0;\n"
	    "	} else {\n"
	    "		g = 1.0 + 4.0 * (vmin + 0.75 * dv - v) / dv;\n"
	    "		b = 0.0;\n"
	    "	}\n"
	    "    return vec4(r, g, b, 0.1);\n"
	    "}\n"
	    "void main() {\n"
		"  //Simulation on GPU \n"
	    "  float x_data = vPosition.x;\n"
	    "  float y_data = vPosition.y;\n"
	    "  float sigma2 = sigma*sigma;\n"
	    "  float z = exp(-0.5*(x_data*x_data)/(sigma2)-0.5*(y_data*y_data)/(sigma2));\n"
	    "  vec4 position = vPosition;\n"
		// scale the
		"  position.z = z*scale;\n"
		"  position.x = position.x*scale;\n"
		"  position.y = position.y*scale;\n"
		"  gl_Position = MVP*position;\n"
	    "  color_based_on_position = heatMap(position.z, 0.0, 0.5);\n"
		"  gl_PointSize = 5.0*scale;\n"
	    "}\n";

	// fragment shader source code
	static const char g_fshader_code_overlay[] =
		"#version 300 es\n"
	    "precision mediump float;\n"
	    "in vec4 color_based_on_position;\n"
		"out vec4 color;\n"
	    "void main() {\n"
	    "  color = color_based_on_position;\n"
	    "}\n";

	//setup the shader for the overlay
	gProgramOverlay = shader.createShaderProgram(g_vshader_code_overlay, g_fshader_code_overlay);
	if (!gProgramOverlay) {
		LOGE("Could not create program for overlay.");
		return false;
	}
	//get handlers for the overlay side
	matrixHandle = glGetUniformLocation(gProgramOverlay, "MVP");
	shader.checkGlError("glGetUniformLocation");
	LOGI("glGetUniformLocation(\"MVP\") = %d\n",
			matrixHandle);

	gvOverlayPositionHandle = glGetAttribLocation(gProgramOverlay, "vPosition");
	shader.checkGlError("glGetAttribLocation");
	LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
			gvOverlayPositionHandle);

	sigmaHandle = glGetUniformLocation(gProgramOverlay, "sigma");
	shader.checkGlError("glGetUniformLocation");
	LOGI("glGetUniformLocation(\"sigma\") = %d\n",
			sigmaHandle);

	scaleHandle = glGetUniformLocation(gProgramOverlay, "scale");
	shader.checkGlError("glGetUniformLocation");
	LOGI("glGetUniformLocation(\"scale\") = %d\n",
			scaleHandle);

	computeGrid();
}

void AROverlayRenderer::setScale(float s) {
	scale = s;
}
void AROverlayRenderer::setScreenSize(int w, int h) {
	width = w;
	height = h;
}

void AROverlayRenderer::setRotMatrix(glm::mat4 r_matrix){
	rotMatrix= r_matrix;
}

void AROverlayRenderer::setOldRotMatrix(glm::mat4 r_matrix){
	old_rotMatrix = r_matrix;
}

void AROverlayRenderer::resetRotMatrix(){
	old_rotMatrix = rotMatrix;
}
void AROverlayRenderer::setDxDy(float dx, float dy){
	//update the angle of rotation for each
	rotX += dx/width;
	rotY += dy/height;
}

/**
 * Compute the projection and view matrices based on camera parameters
 */
void AROverlayRenderer::computeProjectionMatrices(){
	//direction vector for z
	glm::vec3 direction_z(0.0, 0.0, -1.0);
	//up vector
	glm::vec3 up = glm::vec3(0.0, -1.0, 0.0);

	float aspect_ratio = (float)width/(float)height;
	float nearZ = 0.01f;
	float farZ = 50.0f;
	float top = tan(g_initial_fov/2*nearZ);
	float right = aspect_ratio*top;
	float left = -right;
	float bottom = -top;
	g_projection_matrix = glm::frustum(left, right, bottom, top, nearZ, farZ);

	g_view_matrix       = glm::lookAt(
			g_position,           // camera position
			g_position+direction_z, //viewing direction
			up                  // up direction
			);
}


//push graphics on the screen
void AROverlayRenderer::render(){
	//update the variables for animation
	sigma+=0.002f;
	if(sigma>0.5f){
		sigma = 0.002f;
	}

	//render the overlay but enabling the shader program
	glUseProgram(gProgramOverlay);

	//draw the overlay part next
	//get the View and Model matrices and apply to the rendering
	computeProjectionMatrices();

	glm::mat4 projection_matrix = g_projection_matrix;
	glm::mat4 view_matrix = g_view_matrix;
	glm::mat4 model_matrix = glm::mat4(1.0);

	model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, scale-5.0f));
	//X,Y reversed for the screen orientation
	model_matrix = glm::rotate(model_matrix, rotY*glm::pi<float>(), glm::vec3(-1.0f, 0.0f, 0.0f));
	model_matrix = glm::rotate(model_matrix, rotX*glm::pi<float>(), glm::vec3(0.0f, -1.0f, 0.0f));
	model_matrix = glm::rotate(model_matrix, 90.0f*glm::pi<float>()/180.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	//the inverse of rotational matrix is to counter-rotate the
	//graphics to the center. This allows us to reset the
	//camera orientation since R*inv(R) = I.
	view_matrix = rotMatrix*glm::inverse(old_rotMatrix)*view_matrix;

	//create the MVP (model view projection) matrix
	glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

	glUniformMatrix4fv(matrixHandle, 1, GL_FALSE, &mvp[0][0]);
	shader.checkGlError("glUniformMatrix4fv");

	glEnableVertexAttribArray(gvOverlayPositionHandle);
	shader.checkGlError("glEnableVertexAttribArray");

	glVertexAttribPointer(gvOverlayPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, gGrid);
	shader.checkGlError("glVertexAttribPointer");

	glUniform1f(sigmaHandle, sigma);
	shader.checkGlError("glUniform1f");

	glUniform1f(scaleHandle, 1.0f);
	shader.checkGlError("glUniform1f");

	//draw the overlay graphics
	glDrawArrays(GL_POINTS, 0, grid_size*grid_size);
	shader.checkGlError("glDrawArrays");

	glDisableVertexAttribArray(gvOverlayPositionHandle);
}
