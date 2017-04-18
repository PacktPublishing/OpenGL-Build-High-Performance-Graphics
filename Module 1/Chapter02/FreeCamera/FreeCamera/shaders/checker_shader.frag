#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment output colour

//input from the vertex shader
smooth in vec2 vUV;				//interpolated 2D texture coordinates

//uniform
uniform sampler2D textureMap;	//texture map


void main()
{
	//use the interpolated textrue coordinate to lookup the colour from the given texture map
	vFragColor = texture(textureMap, vUV).rrrr;
}