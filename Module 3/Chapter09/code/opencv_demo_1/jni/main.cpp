/*
 * Chapter 9 - OpenGL ES 3 + OpenCV + Android Sensory Manager for Augmented Reality
 * Authors: Raymond Lo and William Lo
 */

#define GLM_FORCE_RADIANS

//header for JNI
#include <jni.h>
#include <android/log.h>
#include <pthread.h>

//header for the OpenGL ES3 library
#include <GLES3/gl3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include <Texture.hpp>
#include <Shader.hpp>
#include <VideoRenderer.hpp>

//include opencv headers
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

#define  LOG_TAG    "libgl3jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//mutex lock for data copying
pthread_mutex_t count_mutex;

int width = 1280;
int height = 720;

//pre-set image size
const int IMAGE_WIDTH = 1280;
const int IMAGE_HEIGHT = 720; 

float scale = 1.0f;
float aspect_ratio=1.0f;
float aspect_ratio_frame=1.0f;

bool enable_process = true;

//all shader related code
Shader shader;
//for video rendering
VideoRenderer videorenderer;

//main camera feed from the Java side
cv::Mat frame;

/**
 * Initialization and call upon changes to graphics framebuffer.
 */
bool setupGraphics(int w, int h) {
	shader.printGLString("Version", GL_VERSION);
	shader.printGLString("Vendor", GL_VENDOR);
	shader.printGLString("Renderer", GL_RENDERER);
	shader.printGLString("Extensions", GL_EXTENSIONS);

	LOGI("setupGraphics(%d, %d)", w, h);
	
	videorenderer.setup();

	glViewport(0, 0, w, h);
	shader.checkGlError("glViewport");

	width = w;
	height = h;
	aspect_ratio = (float)w/(float)h;
	
	//template for the first texture
	cv::Mat frameM(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC4, cv::Scalar(0,0,0,255));
	videorenderer.initTexture(frameM);
	frame = frameM;

	//finally we enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return true;
}

void processFrame(cv::Mat *frame_local){
        int maxCorners = 1000;
        if( maxCorners < 1 ) { maxCorners = 1; }
        cv::RNG rng(12345);
        /// Parameters for Shi-Tomasi algorithm
        std::vector<cv::Point2f> corners;
        double qualityLevel = 0.05;
        double minDistance = 10;
        int blockSize = 3;
        bool useHarrisDetector = false;
        double k = 0.04;

        /// Copy the source image
        cv::Mat src_gray;
        cv::Mat frame_small;
        cv::resize(*frame_local, frame_small, cv::Size(), 0.5, 0.5, CV_INTER_AREA);
        cv::cvtColor(frame_small, src_gray, CV_RGB2GRAY );
        
        /// Apply feature extraction
        cv::goodFeaturesToTrack( src_gray,
                                           corners,
                                           maxCorners,
                                           qualityLevel,
                                           minDistance,
                                           cv::Mat(),
                                           blockSize,
                                           useHarrisDetector,
                                           k );
        
        // Draw corners detected on the image
        int r = 10;
        for( int i = 0; i < corners.size(); i++ )
        {
                cv::circle(*frame_local, 2*corners[i], r, cv::Scalar(rng.uniform(0,255), 
			   rng.uniform(0,255), rng.uniform(0,255), 255), -1, 8, 0 );
        }
        //LOGI("Found %d features", corners.size());
}


/**
 * Calls per render, perform graphics updates
 */
void renderFrame() {
	shader.checkGlError("glClearColor");
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	shader.checkGlError("glClear");
	 
        pthread_mutex_lock(&count_mutex);
	cv::Mat frame_local = frame.clone();
	pthread_mutex_unlock(&count_mutex);

	if(enable_process)
		processFrame(&frame_local);
	
	//render the video feed on the screen
	videorenderer.render(frame_local);
	
	//LOGI("Rendering OpenGL Graphics");
}

//external calls for Java
extern "C" {
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_setScale(JNIEnv * env, jobject obj,  jfloat jscale);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_resetRotDataOffset(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_setRotMatrix (JNIEnv *env, jobject obj, jfloatArray ptr);
 	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_setImage(JNIEnv * jenv, jobject, jlong imageRGBA);
	//toggle features
 	JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_toggleFeatures(JNIEnv * jenv, jobject);
};

//link to internal calls
JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_init(JNIEnv * env, jobject obj, jint width, jint height)
{
	setupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_step(JNIEnv * env, jobject obj)
{
	renderFrame();
}

JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_setScale(JNIEnv * env, jobject obj, jfloat jscale)
{
	LOGI("Scale is %lf", scale);
}

JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_resetRotDataOffset(JNIEnv * env, jobject obj){
}

JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_toggleFeatures(JNIEnv * env, jobject obj){
	//toggle the processing on/off 
	enable_process = !enable_process;
}

JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_setRotMatrix (JNIEnv *env, jobject obj, jfloatArray ptr) {
	jsize len = env->GetArrayLength(ptr);
	jfloat *body = env->GetFloatArrayElements(ptr,0);
	//should be 16 elements from the rotation matrix
	glm::mat4 rotMatrix(1.0f);
	int count = 0;
	for(int i = 0; i<4; i++){
		for(int j=0; j<4; j++){
			rotMatrix[i][j] = body[count];
			count++;
		}
	}
	env->ReleaseFloatArrayElements(ptr, body, 0);
}

JNIEXPORT void JNICALL Java_com_android_gl3jni_GL3JNILib_setImage(
		JNIEnv * jenv, jobject, jlong imageRGBA) {
	cv::Mat* image = (cv::Mat*) imageRGBA;
	//use mutex lock to ensure the write/read operations are synced (to avoid corrupting the frame)
	pthread_mutex_lock(&count_mutex);
	frame = image->clone();
	pthread_mutex_unlock(&count_mutex);
	//LOGI("Got Image: %dx%d\n", frame.rows, frame.cols);
}


