#version 330 core

layout(location=0) out vec4 vFragColor; //fragment shader output
smooth in vec2 vUV;	//input intepolated texture coordinate from vertex shader

uniform sampler2D textureMap;	//the texture map uniform

void main()
{ 
	//sample the texture map at the given texture coordinates
	vFragColor =  texture(textureMap, vUV);
}