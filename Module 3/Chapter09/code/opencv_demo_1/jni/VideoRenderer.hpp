/*
 * VideoRenderer.h
 *
 */

#ifndef VIDEORENDERER_H_
#define VIDEORENDERER_H_
//handles the shader program and basic opengl calls
#include <Shader.hpp>
//for texture supports
#include <Texture.hpp>

//opencv supports
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class VideoRenderer {
public:
	VideoRenderer();
	virtual ~VideoRenderer();
	//setup all shader program and texture mapping variables
	bool setup();
	//render the frame on screen
	void render(cv::Mat frame);
	bool initTexture(cv::Mat frame);

private:
	//this handles the generic camera feed view
	GLuint gProgram;
	GLuint gvPositionHandle;
	GLuint vertexUVHandle;
	GLuint textureSamplerID;
	GLuint texture_id;
	Shader shader;
	Texture texture;
};

#endif /* VIDEORENDERER_H_ */
