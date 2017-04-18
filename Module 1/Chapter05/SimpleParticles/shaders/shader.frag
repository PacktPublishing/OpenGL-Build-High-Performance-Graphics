#version 330 core

layout(location=0) out vec4 vFragColor; // fragment shader output

//input from the vertex shader
smooth in vec4 vSmoothColor;	//lienarly interpolated particle colour


void main()
{
	//use the particle smooth colour as fragment output
	vFragColor = vSmoothColor; 
}