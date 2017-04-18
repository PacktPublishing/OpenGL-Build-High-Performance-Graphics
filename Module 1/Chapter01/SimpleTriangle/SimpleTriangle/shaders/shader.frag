#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output

//input form the vertex shader
smooth in vec4 vSmoothColor;		//interpolated colour to fragment shader

void main()
{
	//set the interpolated colour as the shader output
	vFragColor = vSmoothColor;
}