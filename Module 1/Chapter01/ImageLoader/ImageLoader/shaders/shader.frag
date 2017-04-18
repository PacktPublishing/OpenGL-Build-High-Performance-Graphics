#version 330 core
 
layout (location=0) out vec4 vFragColor;	//fragment shader output

//input from the vertex shader
smooth in vec2 vUV;		//2D texture coordinates

//shader uniform
uniform sampler2D textureMap;		//the image to display

void main()
{
	//sample the textureMap at the given 2D texture coodinates to obntain the colour
	vFragColor =   texture(textureMap, vUV);
}