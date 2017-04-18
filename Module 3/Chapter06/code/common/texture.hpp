#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "common.h"

//Load an image file to the texture
//(supports all image formats offered by the SOIL library)
GLuint loadRGBImageToTexture(const unsigned char *image_data, int width, int height);
GLuint loadImageToTexture(const char * imagepath, int *width, int *height);
GLuint initializeTexture(const unsigned char *image_data, int width, int height, GLenum format=GL_RGBA);
void updateTexture(const unsigned char *image_data, int width, int height, GLenum format=GL_RGBA);

#endif
