/*
 * VideoRenderer.cpp
 *
 */

#include "VideoRenderer.hpp"

#define  LOG_TAG    "VideoRenderer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

VideoRenderer::VideoRenderer() {
	//allocate memory if needed
}

VideoRenderer::~VideoRenderer() {
	//deallocate memory if needed
}

bool VideoRenderer::setup(){
	// Vertex shader source code
	const char g_vshader_code[] =
		"#version 300 es\n"
	    "layout(location = 1) in vec4 vPosition;\n"
		"layout(location = 2) in vec2 vertexUV;\n"
		"out vec2 UV;\n"
		"void main() {\n"
		"  gl_Position = vPosition;\n"
		"  UV=vertexUV;\n"
	    "}\n";

	// fragment shader source code
	const char g_fshader_code[] =
		"#version 300 es\n"
	    "precision mediump float;\n"
		"out vec4 color;\n"
		"uniform sampler2D textureSampler;\n"
		"in vec2 UV;\n"
	    "void main() {\n"
	    "  color = vec4(texture(textureSampler, UV).rgba);\n"
	    "}\n";

	LOGI("setupVideoRenderer");
	gProgram = shader.createShaderProgram(g_vshader_code, g_fshader_code);
	if (!gProgram) {
		LOGE("Could not create program.");
		return false;
	}

	gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
	shader.checkGlError("glGetAttribLocation");
	LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
			gvPositionHandle);

	vertexUVHandle = glGetAttribLocation(gProgram, "vertexUV");
	shader.checkGlError("glGetAttribLocation");
	LOGI("glGetAttribLocation(\"vertexUV\") = %d\n",
			vertexUVHandle);

	textureSamplerID = glGetUniformLocation(gProgram, "textureSampler");
	shader.checkGlError("glGetUniformLocation");
	LOGI("glGetUniformLocation(\"textureSampler\") = %d\n",
			textureSamplerID);

	return true;
}

/**
 *
 */
bool VideoRenderer::initTexture(cv::Mat frame){
	texture_id = texture.initializeTexture(frame.data, frame.size().width, frame.size().height);
	//binds our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(textureSamplerID, 0);

	return true;
}

/**
 * Renderer the frame onto screen using texture mapping
 */
void VideoRenderer::render(cv::Mat frame){
	//our vertices
	const GLfloat g_vertex_buffer_data[] = {
		1.0f,1.0f,0.0f,
		-1.0f,1.0f,0.0f,
		-1.0f,-1.0f,0.0f,
		1.0f,1.0f
		,0.0f,
		-1.0f,-1.0f,0.0f,
		1.0f,-1.0f,0.0f
	};
	//UV map for the vertices
	const GLfloat g_uv_buffer_data[] = {
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};

    glUseProgram(gProgram);
    shader.checkGlError("glUseProgram");

    glEnableVertexAttribArray(gvPositionHandle);
    shader.checkGlError("glEnableVertexAttribArray");

    glEnableVertexAttribArray(vertexUVHandle);
    shader.checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, g_vertex_buffer_data);
    shader.checkGlError("glVertexAttribPointer");

    glVertexAttribPointer(vertexUVHandle, 2, GL_FLOAT, GL_FALSE, 0, g_uv_buffer_data);
    shader.checkGlError("glVertexAttribPointer");

    texture.updateTexture(frame.data, frame.size().width, frame.size().height, GL_RGBA);

	//draw the camera feed on the screen
    glDrawArrays(GL_TRIANGLES, 0, 6);
    shader.checkGlError("glDrawArrays");

    glDisableVertexAttribArray(gvPositionHandle);
    glDisableVertexAttribArray(vertexUVHandle);
}
