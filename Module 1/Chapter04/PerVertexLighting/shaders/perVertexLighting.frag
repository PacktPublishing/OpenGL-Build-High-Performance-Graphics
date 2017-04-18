#version 330 core

layout(location=0) out vec4 vFragColor; //fragment shader output
 
smooth in vec4 color; //interpolated colour from the vertex shader

void main() { 
	//set the input colour from the vertex shader as fragment shader output
	vFragColor = color;	 
}