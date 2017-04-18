/*
 * Chapter 8 - Interactive Real-time Data Visualization on Mobile Devices
 * Authors: Raymond Lo and William Lo
 *
 * HOW_IT_WORKS:
    In this demo, we explore the use of a shader program written in OpenGL ES 3.0 to perform all the
    simulation and heat map-based 3-D rendering steps to visualize a Gaussian distribution on a 
    mobile GPU. At first sight, the shader code in OpenGL ES 3.0 seems to show no significant
    differences compared with the code written in standard OpenGL 3.x and above. However, we 
    recommend that you consult the specification to ensure that a particular feature of interest co-
    exists in both versions. More details on the OpenGL ES 3.0 specifications can be found here:
    https://www.khronos.org/registry/gles/specs/3.0/es_spec_3.0.0.pdf
 
    The hardware-accelerated portion of the code is programmed within the vertex shader program and 
    stored inside the g_vshader_code variable. The vertex program handles the computation related to 
    the simulation (in our case, we have a Gaussian function with a time-varying sigma value) in the 
    graphics hardware. We pass in the sigma value as a uniform variable and it is used to compute 
    the surface height. In addition, we also compute the heat map color value within the shader 
    program based on the height value. With this approach, we have significantly improved the speed 
    of the graphics rendering step by completely eliminating the use of the CPU cycles on these large 
    number of floating point operations.
 
    In addition, we have ported the GLM Library used in previous chapters to the Android platform by 
    including the headers as well as including the GLM path in the build script Android.mk. The GLM 
    library handles the view and projection matrix computation and also allows us to migrate most of 
    our previous work, such as stereoscopic 3D rendering demonstrated in an earlier chapter to the 
    Android platform.
 
    Finally, our Android-based application also utilizes the inputs from the multi-touch screen 
    interface and device orientation derived from the motion sensor data. These values are passed 
    through the JNI directly to the shader program as uniform variables.
 *
 */

#define GLM_FORCE_RADIANS

//header for JNI
#include <jni.h>
#include <android/log.h>

//header for the OpenGL ES3 library
#include <GLES3/gl3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//header for GLM library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define  LOG_TAG    "libgl3jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//shader programs and all variables handlers
GLuint gProgram;
GLuint gvPositionHandle;
GLuint matrixHandle;
GLuint sigmaHandle;
GLuint scaleHandle;

//vertices for the grid
const unsigned int GRID_SIZE=400;
GLfloat gGrid[GRID_SIZE*GRID_SIZE*3]={0};

//the view matrix and projection matrix
glm::mat4 g_view_matrix;
glm::mat4 g_projection_matrix;

//initial position of the camera
glm::vec3 g_position = glm::vec3( 0, 0, 4 );

//FOV of the camera
float g_initial_fov = glm::pi<float>()*0.25f;
//rotation angle, set by sensors or by touch screen
float rx, ry, rz;
float scale=1.0f;

int width = 640;
int height = 480;

// Vertex shader source code
static const char g_vshader_code[] =
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
	"  position.z = z*scale;\n"
	"  position.x = position.x*scale;\n"
	"  position.y = position.y*scale;\n"
	"  gl_Position = MVP*position;\n"
    "  color_based_on_position = heatMap(position.z, 0.0, 0.5);\n"
	"  gl_PointSize = 5.0*scale;\n"
    "}\n";

// fragment shader source code
static const char g_fshader_code[] =
	"#version 300 es\n"
    "precision mediump float;\n"
    "in vec4 color_based_on_position;\n"
	"out vec4 color;\n"
    "void main() {\n"
    "  color = color_based_on_position;\n"
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
 * Initialize the grid pattern vertices for GPU simulation
 */
void computeGrid(){
	float grid_x = GRID_SIZE;
	float grid_y = GRID_SIZE;
	unsigned int data_counter = 0;
	//define a grid ranging from -1 to +1
	for(float x = -grid_x/2.0f; x<grid_x/2.0f; x+=1.0f){
		for(float y = -grid_y/2.0f; y<grid_y/2.0f; y+=1.0f){
			float x_data = 2.0f*x/grid_x;
			float y_data = 2.0f*y/grid_y;
			gGrid[data_counter] = x_data;
			gGrid[data_counter+1] = y_data;
			gGrid[data_counter+2] = 0;
			data_counter+=3;
		}
	}
}

/**
 * Compute the projection and view matrices based on camera parameters
 */
void computeProjectionMatrices(){
	//direction vector for z 
	glm::vec3 direction_z(0, 0, -1.0);
	//up vector
	glm::vec3 up = glm::vec3(0,-1,0);
	
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
			g_position+direction_z, //view direction
			up                  // up direction
			);
}

/**
 * Set the angles for rotation
 */
void setAngles(float irx, float iry, float irz){
	rx = irx;
	ry = iry;
	rz = irz;
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

    matrixHandle = glGetUniformLocation(gProgram, "MVP");
    checkGlError("glGetUniformLocation");
    LOGI("glGetUniformLocation(\"MVP\") = %d\n",
    		matrixHandle);

    sigmaHandle = glGetUniformLocation(gProgram, "sigma");
    checkGlError("glGetUniformLocation");
    LOGI("glGetUniformLocation(\"sigma\") = %d\n",
    		sigmaHandle);

    scaleHandle = glGetUniformLocation(gProgram, "scale");
    checkGlError("glGetUniformLocation");
    LOGI("glGetUniformLocation(\"scale\") = %d\n",
    		scaleHandle);

    glViewport(0, 0, w, h);
    width = w;
    height = h;

    checkGlError("glViewport");

    computeGrid();
    return true;
}

/**
 * Calls per render, perform graphics updates
 */
void renderFrame() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    static float sigma;

    //update the variables for animations
    sigma+=0.002f;
    if(sigma>0.5f){
       sigma = 0.002f;
    }

    //get the View and Model Matrix and apply to the rendering
    computeProjectionMatrices();
    glm::mat4 projection_matrix = g_projection_matrix;
    glm::mat4 view_matrix = g_view_matrix;
    glm::mat4 model_matrix = glm::mat4(1.0);
    model_matrix = glm::rotate(model_matrix, rz, glm::vec3(-1.0f, 0.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, ry, glm::vec3(0.0f, -1.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, rx, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    checkGlError("glClearColor");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");
    
    glUniformMatrix4fv(matrixHandle, 1, GL_FALSE, &mvp[0][0]);
    checkGlError("glUniformMatrix4fv");

    glUniform1f(sigmaHandle, sigma);
    checkGlError("glUniform1f");

    glUniform1f(scaleHandle, scale);
    checkGlError("glUniform1f");

    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, gGrid);
    checkGlError("glVertexAttribPointer");

    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");

    glDrawArrays(GL_POINTS, 0, GRID_SIZE*GRID_SIZE);
    checkGlError("glDrawArrays");
}

//external calls for Java
extern "C" {
    JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addRotData(JNIEnv * env, jobject obj,  jfloat rx, jfloat ry, jfloat rz);
    JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_setScale(JNIEnv * env, jobject obj,  jfloat jscale);
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
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_addRotData(JNIEnv * env, jobject obj, jfloat rx, jfloat ry, jfloat rz)
{
    setAngles(rx, ry, rz);
}
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_setScale(JNIEnv * env, jobject obj, jfloat jscale)
{
    scale = jscale;
    LOGI("Scale is %lf", scale);
}

