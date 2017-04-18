#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "common.h"

//Load an image file to the texture
//(supports all image formats offered by the SOIL library)
GLuint loadImageToTexture(const char * imagepath, int *width, int *height);
GLuint initializeTexture(const unsigned char *image_data, int width, int height, GLenum format);
void updateTexture(const unsigned char *image_data, int width, int height, GLenum format);

#endif
