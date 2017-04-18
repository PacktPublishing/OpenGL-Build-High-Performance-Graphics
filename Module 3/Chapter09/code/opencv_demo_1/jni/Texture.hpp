#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GLES3/gl3.h>

//Load an image file to the texture
class Texture {
public:
	Texture();
	virtual ~Texture();
	GLuint initializeTexture(const unsigned char *image_data, int width, int height);
	void updateTexture(const unsigned char *image_data, int width, int height, GLenum format);
};

#endif
