/*
 * Shader.cpp
 *
 */

#include "Shader.hpp"
#define  LOG_TAG    "shader"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

Shader::Shader() {
}

Shader::~Shader() {
}

/**
 * Print out the error string from OpenGL
 */
void Shader::printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

/**
 * Error checking with OpenGL calls
 */
void Shader::checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("After %s() glError (0x%x)\n", op, error);
    }
}

/**
 * Load shader program and return the id
 */
GLuint Shader::loadShader(GLenum shader_type, const char* p_source) {
    GLuint shader = glCreateShader(shader_type);
    if (shader) {
        glShaderSource(shader, 1, &p_source, 0);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        //Report error and delete the shader
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, 0, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shader_type, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

/**
 * Create the shader program with vertex and fragment shader
 */
GLuint Shader::createShaderProgram(const char *vertex_shader_code, const char *fragment_shader_code){
	//create the vertex and fragment shaders
	GLuint vertex_shader_id = loadShader(GL_VERTEX_SHADER, vertex_shader_code);
	if (!vertex_shader_id) {
		return 0;
	}

	GLuint fragment_shader_id = loadShader(GL_FRAGMENT_SHADER, fragment_shader_code);
    if (!fragment_shader_id) {
        return 0;
    }

	GLint result = GL_FALSE;
	//link the program
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
    checkGlError("glAttachShader");
	glAttachShader(program_id, fragment_shader_id);
    checkGlError("glAttachShader");
	glLinkProgram(program_id);
	//check the program and ensure that the program is linked properly
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	if ( result != GL_TRUE ){
		//error handling with Android
		GLint bufLength = 0;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &bufLength);
		if (bufLength) {
			char* buf = (char*) malloc(bufLength);
			if (buf) {
				glGetProgramInfoLog(program_id, bufLength, 0, buf);
				LOGE("Could not link program:\n%s\n", buf);
				free(buf);
			}
		}
		glDeleteProgram(program_id);
		program_id = 0;
	}else{
		LOGI("Linked program Successfully\n");
	}

	//flag for delete, and will free all memories
	//when the attached program is deleted
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return program_id;
}
