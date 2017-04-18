//=======================================================================
// Authors: Raymond Lo and William Lo
//=======================================================================

#include "Texture.hpp"

Texture::Texture() {
	//allocate memory here if needed
}

Texture::~Texture() {
	//deallocate memory here if needed
}

GLuint Texture::initializeTexture(const unsigned char *image_data, int width, int height){
	GLuint textureID=0;

	//for the first time we create the image
	//create one texture element
	glGenTextures(1, &textureID);

	//bind the one element
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Very complicated concept, will leave it for later explanation
	//http://opengl.czweb.org/ch11/358-361.html
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	//Specifies the target texture and the parameters describe the format and type of the image data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

	//Sets the wrap parameter for texture coordinate s & t to GL_CLAMP_TO_EDGE,
	//This will cause the coordinates to be clamped within the range [0, 1].
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Sets the magnification method to linear and will return an weighted average of four texture elements
	//that are closest to the center of the pixel being texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Chooses the mipmap that most closely matches the size of the pixel being textured and uses the GL_NEAREST
	//criterion (the texture element nearest to the center of the pixel) to produce a texture value.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

//push new data update to the texture
void Texture::updateTexture(const unsigned char *image_data, int width, int height, GLenum format){
	// Update The Texture
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, image_data);
	//Sets the wrap parameter for texture coordinate s & t to GL_CLAMP,
	//This will cause the coordinates to be clamped within the range [0, 1].
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Sets the magnification method to linear and will return an weighted average of four texture elements
	//that are closest to the center of the pixel being texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Chooses the mipmap that most closely matches the size of the pixel being textured and uses the GL_NEAREST
	//criterion (the texture element nearest to the center of the pixel) to produce a texture value.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}
