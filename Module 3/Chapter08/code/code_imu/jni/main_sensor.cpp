/*
 * Chapter 8 - Interactive Real-time Data Visualization on Mobile Devices
 * Visualizing real-time data from built-in inertial measurement units (IMUs)
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

#include <Sensor.h>


#define  LOG_TAG    "libgl3jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//shader program handlers and other variables
GLuint gProgram;

GLuint gxPositionHandle;
GLuint gyPositionHandle;
GLuint gColorHandle;
GLuint gOffsetHandle;
GLuint gScaleHandle;

static Sensor g_sensor_data;

int width = 640;
int height = 480;

// Vertex shader source code
static const char g_vshader_code[] =
"#version 300 es\n"
"in float yPosition;\n"
"in float xPosition;\n"
"uniform float scale;\n"
"uniform float offset;\n"
"void main() {\n"
"  vec4 position = vec4(xPosition, yPosition*scale+offset, 0.0, 1.0);\n"
"  gl_Position = position;\n"
"}\n";

// fragment shader source code
static const char g_fshader_code[] =
"#version 300 es\n"
"precision mediump float;\n"
"uniform vec4 color;\n"
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
 * Create the shader program with vertex and fragment shaders
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
	gxPositionHandle = glGetAttribLocation(gProgram, "xPosition");
	checkGlError("glGetAttribLocation");
	LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
			gxPositionHandle);

	gyPositionHandle = glGetAttribLocation(gProgram, "yPosition");
	checkGlError("glGetAttribLocation");
	LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
			gyPositionHandle);

	gColorHandle = glGetUniformLocation(gProgram, "color");
	checkGlError("glGetUniformLocation");
	LOGI("glGetUniformLocation(\"color\") = %d\n",
			gColorHandle);

	gOffsetHandle = glGetUniformLocation(gProgram, "offset");
	checkGlError("glGetUniformLocation");
	LOGI("glGetUniformLocation(\"offset\") = %d\n",
			gOffsetHandle);

	gScaleHandle = glGetUniformLocation(gProgram, "scale");
	checkGlError("glGetUniformLocation");
	LOGI("glGetUniformLocation(\"scale\") = %d\n",
			gScaleHandle);

	glViewport(0, 0, w, h);
	width = w;
	height = h;

	checkGlError("glViewport");

	return true;
}



void draw2DPlot(GLfloat *data, unsigned int size, GLfloat scale, GLfloat offset){
	glVertexAttribPointer(gyPositionHandle, 1, GL_FLOAT, GL_FALSE, 0, data);
	checkGlError("glVertexAttribPointer");

	glEnableVertexAttribArray(gyPositionHandle);
	checkGlError("glEnableVertexAttribArray");

	glUniform1f(gOffsetHandle, offset);
	checkGlError("glUniform1f");

	glUniform1f(gScaleHandle, scale);
	checkGlError("glUniform1f");

	glDrawArrays(GL_LINE_STRIP, 0, g_sensor_data.getBufferSize());
	checkGlError("glDrawArrays");
}


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

	glVertexAttribPointer(gxPositionHandle, 1, GL_FLOAT, GL_FALSE, 0, g_sensor_data.getAxisPtr());
	checkGlError("glVertexAttribPointer");

	glEnableVertexAttribArray(gxPositionHandle);
	checkGlError("glEnableVertexAttribArray");

	//Obtain the scaling factor based on the dataset
	//0.33f for 1/3 of the screen for each graph
	float acc_scale = 0.33f/g_sensor_data.getAccScale();
	float gyro_scale = 0.33f/g_sensor_data.getGyroScale();
	float mag_scale = 0.33f/g_sensor_data.getMagScale();

	glLineWidth(4.0f);

	//set the rendering color
	glUniform4f(gColorHandle, 1.0f, 0.0f, 0.0f, 1.0f);
	checkGlError("glUniform1f");

	//render the accelerometer, gyro, and digital compass data
	//As the vertex shader does not use any projection matrix, every visible vertex 
	//has to be in the range of [-1, 1]. 
	//0.67f, 0.0f, and -0.67f are the vertical position of each graph
	draw2DPlot(g_sensor_data.getAccelDataPtr(0), g_sensor_data.getBufferSize(), acc_scale, 0.67f);
	draw2DPlot(g_sensor_data.getGyroDataPtr(0), g_sensor_data.getBufferSize(), gyro_scale, 0.0f);
	draw2DPlot(g_sensor_data.getMagDataPtr(0), g_sensor_data.getBufferSize(), mag_scale, -0.67f);

	glUniform4f(gColorHandle, 0.0f, 1.0f, 0.0f, 1.0f);
	checkGlError("glUniform1f");
	draw2DPlot(g_sensor_data.getAccelDataPtr(1), g_sensor_data.getBufferSize(), acc_scale, 0.67f);
	draw2DPlot(g_sensor_data.getGyroDataPtr(1), g_sensor_data.getBufferSize(), gyro_scale, 0.0f);
	draw2DPlot(g_sensor_data.getMagDataPtr(1), g_sensor_data.getBufferSize(), mag_scale, -0.67f);

	glUniform4f(gColorHandle, 0.0f, 0.0f, 1.0f, 1.0f);
	checkGlError("glUniform1f");
	draw2DPlot(g_sensor_data.getAccelDataPtr(2), g_sensor_data.getBufferSize(), acc_scale, 0.67f);
	draw2DPlot(g_sensor_data.getGyroDataPtr(2), g_sensor_data.getBufferSize(), gyro_scale, 0.0f);
	draw2DPlot(g_sensor_data.getMagDataPtr(2), g_sensor_data.getBufferSize(), mag_scale, -0.67f);

}

//external calls for Java
extern "C" {
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addAccelData(JNIEnv * env, jobject obj,  jfloat ax, jfloat ay, jfloat az);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addGyroData(JNIEnv * env, jobject obj,  jfloat gx, jfloat gy, jfloat gz);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addMagData(JNIEnv * env, jobject obj,  jfloat mx, jfloat my, jfloat mz);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addRotData(JNIEnv * env, jobject obj,  jfloat rx, jfloat ry, jfloat rz);
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
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addAccelData(JNIEnv * env, jobject obj,  jfloat ax, jfloat ay, jfloat az){
	g_sensor_data.appendAccelData(ax, ay, az);
}
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addGyroData(JNIEnv * env, jobject obj,  jfloat gx, jfloat gy, jfloat gz){
	g_sensor_data.appendGyroData(gx, gy, gz);
}
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addMagData(JNIEnv * env, jobject obj,  jfloat mx, jfloat my, jfloat mz){
	g_sensor_data.appendMagData(mx, my, mz);
}
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addRotData(JNIEnv * env, jobject obj,  jfloat rx, jfloat ry, jfloat rz){
	//	g_sensor_data.appendRotData(rx, ry, rz);
}

