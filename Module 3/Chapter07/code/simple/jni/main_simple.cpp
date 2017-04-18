/*
 * Chapter 7 - OpenGL ES 3 Sample Code
 * Authors: Raymond Lo and William Lo
 */

//header for JNI
#include <jni.h>
#include <android/log.h>

//header for the OpenGL ES3 library
#include <GLES3/gl3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define  LOG_TAG    "libgl3jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//shader programs and all handlers
GLuint gProgram;
GLuint gvPositionHandle;
GLuint gvColorHandle;

int width = 640;
int height = 480;

// Vertex shader source code
static const char g_vshader_code[] =
	"#version 300 es\n"
    "in vec4 vPosition;\n"
	"in vec4 vColor;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "  color = vColor;\n"
    "}\n";

// fragment shader source code
static const char g_fshader_code[] =
	"#version 300 es\n"
    "precision mediump float;\n"
    "in vec4 color;\n"
	"out vec4 color_out;\n"
    "void main() {\n"
    "  color_out = color;\n"
    "}\n";


/**
 * Print out the error string from OpenGL
 */
static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

/**
 * Error checking with OpenGL calls
 */
static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("After %s() glError (0x%x)\n", op, error);
    }
}

/**
 * Load shader program and return the id
 */
GLuint loadShader(GLenum shader_type, const char* p_source) {
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
GLuint createShaderProgram(const char *vertex_shader_code, const char *fragment_shader_code){
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

/**
 * Initialization and call upon changes to graphics framebuffer.
 */
bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createShaderProgram(g_vshader_code, g_fshader_code);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
            gvPositionHandle);

    gvColorHandle = glGetAttribLocation(gProgram, "vColor");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vColor\") = %d\n",
    		gvColorHandle);

    glViewport(0, 0, w, h);
    width = w;
    height = h;

    checkGlError("glViewport");

    return true;
}

//vertices
GLfloat gTriangle[9]={-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f};
GLfloat gColor[9]={1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f};

/**
 * Calls per render, perform graphics updates
 */
void renderFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    checkGlError("glClearColor");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");
    
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, gTriangle);
    checkGlError("glVertexAttribPointer");

    glVertexAttribPointer(gvColorHandle, 3, GL_FLOAT, GL_FALSE, 0, gColor);
    checkGlError("glVertexAttribPointer");

    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");

    glEnableVertexAttribArray(gvColorHandle);
    checkGlError("glEnableVertexAttribArray");

    glDrawArrays(GL_TRIANGLES, 0, 9);
    checkGlError("glDrawArrays");
}

//external calls for Java
extern "C" {
    JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_step(JNIEnv * env, jobject obj);
};

//link to internal calls
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
    setupGraphics(width, height);
}
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_step(JNIEnv * env, jobject obj)
{
    renderFrame();
}
