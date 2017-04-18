/*
 * Shader.h
 *
 */

#ifndef SHADER_H_
#define SHADER_H_

#define GLM_FORCE_RADIANS

#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//header for the OpenGL ES3 library
#include <GLES3/gl3.h>

//header for GLM library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Shader {
public:
	Shader();
	virtual ~Shader();
	GLuint loadShader(GLenum shader_type, const char *p_source);
	GLuint createShaderProgram(const char *vertex_shader_code, const char *fragment_shader_code);
	void printGLString(const char *name, GLenum s) ;
	void checkGlError(const char *op);
};

#endif /* SHADER_H_ */
