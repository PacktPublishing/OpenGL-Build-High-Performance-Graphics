#version 330 core

layout(location=0) out vec4 vFragColor; //fragment shader output

smooth in vec2 vUV;	//interpolated varying from the vertex shader

//uniform
uniform sampler2D textureMap;	//texture map

void main()
{ 
	//sample the texture map at the given texture coordinate
	vFragColor = texture(textureMap, vUV);
}