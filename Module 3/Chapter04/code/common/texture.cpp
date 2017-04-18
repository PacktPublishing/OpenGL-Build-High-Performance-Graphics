//=======================================================================
// Authors: Raymond Lo and William Lo
// Chapter 4 - Modern OpenGL - Texture Mapping with 2D images
// Copyrights & Licenses:
//=======================================================================

#include "texture.hpp"
//for image I/O
#include <SOIL.h>

// Handle loading images to texture memory and setting up the parameters
GLuint loadImageToTexture(const char * imagepath, int *width, int *height){
	int channels;
	GLuint textureID=0;

	//Load the images and convert them to RGBA format
	unsigned char* image =
		SOIL_load_image(imagepath, width, height, &channels, SOIL_LOAD_RGBA);

	if(!image){
		printf("Failed to load image %s\n", imagepath);
		return textureID;
	}
	printf("Loaded Image: %d x %d - %d channels\n", *width, *height, channels);

	textureID=initializeTexture(image, *width, *height, GL_RGBA); 

	SOIL_free_image_data(image);
	return textureID;
}

GLuint initializeTexture(const unsigned char *image_data, int width, int height, GLenum format){
	GLuint textureID=0;

	//for the first time we create the image
	//create one texture element
	glGenTextures(1, &textureID);

	//bind the one element
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Very complicated concept, will leave it for explanation later
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//Specify the target texture and the parameters describe the format and type of the image data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);

	//Set the wrap parameter for texture coordinate s & t to GL_CLAMP,
	//This will cause the coordinates to be clamped within the range [0, 1].
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Set the magnification method to linear and will return an weighted average of four texture elements
	//that are closest to the center of the pixel being texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Choose the mipmap that most closely matches the size of the pixel being textured and uses the GL_NEAREST
	//criterion (the texture element nearest to the center of the pixel) to produce a texture value.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

//push new data update to the texture
void updateTexture(const unsigned char *image_data, int width, int height, GLenum format){
	// Update The Texture
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, image_data);
	//Set the wrap parameter for texture coordinate s & t to GL_CLAMP,
	//This will cause the coordinates to be clamped within the range [0, 1].
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Set the magnification method to linear and will return an weighted average of four texture elements
	//that are closest to the center of the pixel being texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Choose the mipmap that most closely matches the size of the pixel being textured and uses the GL_NEAREST
	//criterion (the texture element nearest to the center of the pixel) to produce a texture value.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}
